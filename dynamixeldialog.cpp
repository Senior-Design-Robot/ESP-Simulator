#include "dynamixeldialog.h"
#include "ui_dynamixeldialog.h"
#include "dynamixel.h"
#include <iomanip>

DynamixelDialog::DynamixelDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DynamixelDialog)
{
    ui->setupUi(this);

    on_refreshPortsButton_clicked();

    serialConn.setBaudRate(QSerialPort::Baud9600);
    serialConn.setDataBits(QSerialPort::Data8);
    serialConn.setStopBits(QSerialPort::OneStop);
    serialConn.setParity(QSerialPort::NoParity);

    recvTimer = new QTimer(this);
    connect(recvTimer, &QTimer::timeout, this, &DynamixelDialog::checkIncoming);
    recvTimer->setInterval(200);
}

DynamixelDialog::~DynamixelDialog()
{
    delete ui;
}

void DynamixelDialog::on_baudSpin_valueChanged(int newVal)
{
    serialConn.setBaudRate(newVal);
}

void DynamixelDialog::on_connectButton_clicked()
{
    on_disconnectButton_clicked();

    serialConn.setPort(*selectedPortInfo);

    bool isOpen = serialConn.open(QIODevice::ReadWrite);

    if( isOpen )
    {
        ui->statusLabel->setText("Open");
        ui->connectButton->setEnabled(false);
        ui->disconnectButton->setEnabled(true);
        ui->sendButton->setEnabled(true);
        recvTimer->start();
    }
}

void DynamixelDialog::on_disconnectButton_clicked()
{
    if( serialConn.isOpen() ) serialConn.close();
    ui->statusLabel->setText("Closed");
    ui->connectButton->setEnabled(true);
    ui->disconnectButton->setEnabled(false);
    ui->sendButton->setEnabled(false);
    recvTimer->stop();
}

void DynamixelDialog::on_refreshPortsButton_clicked()
{
    portList = QSerialPortInfo::availablePorts();

    refreshInProgress = true;

    ui->portCombo->clear();

    for( auto &port : portList )
    {
        QString pName = port.portName();
        ui->portCombo->addItem(pName);
    }

    refreshInProgress = false;
}

void DynamixelDialog::on_portCombo_currentIndexChanged(int index)
{
    if( refreshInProgress ) return;
    if( index >= 0 && index < portList.length() )
    {
        selectedPortInfo = &(portList[index]);
    }
    else
    {
        selectedPortInfo = nullptr;
    }
}

static int pkt_idx = 0;
static int rcv_idx = 0;
const int RCV_BUF_LEN = 128;
static char rcv_buf[RCV_BUF_LEN];

static bool is_valid_header( char *pkt, int len )
{
    if( (uint8_t)pkt[0] != 0xFF ) return false;
    if( len < 2 ) return true;

    if( (uint8_t)pkt[1] != 0xFF ) return false;
    if( len < 3 ) return true;

    if( (uint8_t)pkt[2] != 0xFD ) return false;
    if( len < 4 ) return true;

    if( (uint8_t)pkt[3] == 0xFD ) return false; // byte stuffing, not header
    return true;
}

void DynamixelDialog::checkIncoming()
{
    qint64 avail = serialConn.bytesAvailable();
    if( avail > 0 )
    {
        int maxRead = RCV_BUF_LEN - rcv_idx;
        if( avail > maxRead ) avail = maxRead;

        int nRead = serialConn.read(rcv_buf, avail);
        rcv_idx += nRead;
    }

    int data_len = rcv_idx - pkt_idx;
    if( data_len < 1 ) return; // no data

    while( (data_len > 0) && !is_valid_header(rcv_buf + pkt_idx, data_len) )
    {
        // flush non-matching char to output
        recvStream << rcv_buf[pkt_idx];
        pkt_idx++;
        data_len = rcv_idx - pkt_idx;
    }

    if( data_len > HEADER_LEN )
    {
        // full header detected
        uint8_t *pkt = (uint8_t*)(rcv_buf + pkt_idx);

        // process device status
        uint8_t dev_id = pkt[PKT_ID];

        uint8_t inst = pkt[PKT_INSTRUCT];
        bool status_pkt = inst == INST_STATUS;

        uint16_t pkt_len = get_pkt_short(pkt, PKT_LEN);

        // make sure entire packet is received before processing
        if( rcv_idx - pkt_idx >= HEADER_LEN + pkt_len )
        {
            std::stringstream pkt_rep;

            pkt_rep << std::endl << "[PKT ID=" << std::hex << (int)dev_id << " INST=" << std::hex << (int)inst;
            pkt_rep << " LEN=" << std::dec << (int)pkt_len;
            if( status_pkt )
            {
                pkt_rep << " ERR=" << std::hex << (int)pkt[PKT_ERR];
            }

            int param_len = pkt_len - 3; // length includes INST, and 2B CRC (& ERR if status)
            if( status_pkt ) param_len -= 1;

            pkt_rep << '|';
            for( int i = 1; i <= param_len; i++ )
            {
                int idx = status_pkt ? STAT_PARAM(i) : PARAM(i);
                uint8_t b = pkt[idx];
                pkt_rep << " 0x";
                pkt_rep << std::setfill('0') << std::setw(2) << std::hex << (int)b;
            }

            pkt_rep << "]" << std::endl;

            if( !ui->hidePacketsCheck->isChecked() )
            {
                recvStream << pkt_rep.str();
            }

            size_t next_pkt_offset = HEADER_LEN + pkt_len;
            pkt_idx += next_pkt_offset;
        }
    }

    // shift remaining data in buffer to the front
    if( pkt_idx > 0 )
    {
        int data_len = rcv_idx - pkt_idx;
        if( data_len > 0 )
        {
            // only need to move data if data exists
            memmove(rcv_buf, rcv_buf + pkt_idx, data_len);
        }

        pkt_idx = 0;
        rcv_idx = pkt_idx + data_len;
    }

    std::string s = recvStream.str();
    ui->recvOutput->setText(QString::fromStdString(s));
}

static bool tryConvertHex( char c, char &val )
{
    if( (c >= '0') && (c <= '9') )
    {
        val = c - '0';
        return true;
    }

    if( (c >= 'A') && (c <= 'F') )
    {
        val = c - 'A' + 10;
        return true;
    }

    if( (c >= 'a') && (c <= 'f') )
    {
        val = c - 'a' + 10;
        return true;
    }

    return false;
}

void DynamixelDialog::on_sendButton_clicked()
{
    std::string datStr = ui->xmitDataInput->text().remove(' ').toStdString();
    int byte_len = datStr.length() / 2;
    char data[byte_len];

    for( size_t i = 0; i < datStr.length(); i++ )
    {
        int byte_idx = i / 2;
        char val;

        if( tryConvertHex(datStr.at(i), val) )
        {
            if( !(i % 2) )
            {
                data[byte_idx] = val << 4;
            }
            else
            {
                data[byte_idx] |= (val & 0xFF);
            }
        }
        else
        {
            ui->sendButton->setStyleSheet("color: red");
            return; // convert failed
        }
    }

    serialConn.write(data, byte_len);
    ui->sendButton->setStyleSheet("color: black");
}

void DynamixelDialog::on_clearRecvButton_clicked()
{
    recvStream.clear();
    recvStream.str("");
    ui->recvOutput->setText("");
}
