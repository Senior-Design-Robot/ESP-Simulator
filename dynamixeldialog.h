#pragma once

#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <sstream>

namespace Ui {
class DynamixelDialog;
}

class DynamixelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DynamixelDialog(QWidget *parent = nullptr);
    ~DynamixelDialog();

private slots:
    void on_baudSpin_valueChanged(int newVal);
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_refreshPortsButton_clicked();
    void on_portCombo_currentIndexChanged(int index);
    void checkIncoming();

    void on_sendButton_clicked();

    void on_clearRecvButton_clicked();

private:
    Ui::DynamixelDialog *ui;
    QSerialPort serialConn;
    QList<QSerialPortInfo> portList;
    QSerialPortInfo *selectedPortInfo;

    QTimer *recvTimer;
    std::stringstream recvStream;
    bool lastCharAscii = true;

    bool refreshInProgress = false;
};

