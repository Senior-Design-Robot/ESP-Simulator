#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kinematics.h"
#include "pathiterator.h"
#include <QtMath>
#include <QFileDialog>
#include <fstream>
#include <sstream>


QHostAddress rpi_addr("127.0.0.1");

const int PATH_STATUS_DELAY = 100;
const int IDLE_STATUS_DELAY = 4000;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    moveTimer.setInterval(10);
    connect(&moveTimer, &QTimer::timeout, this, &MainWindow::moveTimer_timeout);

    server1 = new QTcpServer(this);
    connect(server1, &QTcpServer::newConnection, this, &MainWindow::server1_newConnection);
    server1->listen(QHostAddress::AnyIPv4, 1897);

    server2 = new QTcpServer(this);
    connect(server2, &QTcpServer::newConnection, this, &MainWindow::server2_newConnection);
    server2->listen(QHostAddress::AnyIPv4, 1898);

    moveTimer.start();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete server1;
    delete server2;
}


void MainWindow::send_status( int device )
{
    if( device == 1 )
    {
        // pkt_type, id, power, mode, shoulder, elbow, odo, remaining
        QString data = QString("1,%1,1,%2,0,0,0,%3\n").arg(device).arg(arm1_mode).arg(arm1path.remaining());
        TransmitWrapper *w = new TransmitWrapper(this);
        w->start_transmit(rpi_addr, data.toUtf8());
    }
    else
    {

        QString data = QString("1,%1,1,%2,0,0,0,%3\n").arg(device).arg(arm2_mode).arg(arm2path.remaining());
        TransmitWrapper *w = new TransmitWrapper(this);
        w->start_transmit(rpi_addr, data.toUtf8());
    }
}


void MainWindow::moveTimer_timeout()
{
    struct arm_angles ang;
    PathElement nextMove;

    QTime curTime = QTime::currentTime();

    int deltaStatusTime = lastStatus1.msecsTo(curTime);
    bool forcePathStatus = (arm1_mode == MODE_DRAW) && (arm1path.remaining() < MIN_PATH_FILL) && (deltaStatusTime >= PATH_STATUS_DELAY);

    if( forcePathStatus || (deltaStatusTime >= IDLE_STATUS_DELAY) )
    {
        send_status(1);
        lastStatus1 = curTime;
    }

    if( arm1_mode == MODE_DRAW )
    {
        nextMove = arm1path.moveNext();
        switch( nextMove.type )
        {
        case PATH_MOVE:
            ang = calculate_angles(nextMove.x - ARM1_X, nextMove.y);
            ui->armCanvas->setArmPosition(ang, nextMove.x, nextMove.y);
            break;

        case PATH_PEN_UP:
            ui->armCanvas->penDown = false;
            break;

        case PATH_PEN_DOWN:
            ang = calculate_angles(nextMove.x - ARM1_X, nextMove.y);
            ui->armCanvas->setArmPosition(ang, nextMove.x, nextMove.y);
            ui->armCanvas->penDown = true;
            break;

        case PATH_END:
            arm1_mode = MODE_IDLE;
            send_status(1);
            break;

        default:
        case PATH_NONE:
            break;
        }
    }

    // second arm
    deltaStatusTime = lastStatus2.msecsTo(curTime);
    forcePathStatus = (arm2_mode == MODE_DRAW) && (arm2path.remaining() < MIN_PATH_FILL) && (deltaStatusTime >= PATH_STATUS_DELAY);

    if( forcePathStatus || (deltaStatusTime >= IDLE_STATUS_DELAY) )
    {
        send_status(2);
        lastStatus2 = curTime;
    }

    if( arm2_mode == MODE_DRAW )
    {
        nextMove = arm2path.moveNext();
        switch( nextMove.type )
        {
        case PATH_MOVE:
            ang = calculate_angles(nextMove.x - ARM2_X, nextMove.y);
            ui->armCanvas->setArm2Position(ang, nextMove.x, nextMove.y);
            break;

        case PATH_PEN_UP:
            ui->armCanvas->penDown2 = false;
            break;

        case PATH_PEN_DOWN:
            ang = calculate_angles(nextMove.x - ARM2_X, nextMove.y);
            ui->armCanvas->setArm2Position(ang, nextMove.x, nextMove.y);
            ui->armCanvas->penDown2 = true;
            break;

        case PATH_END:
            arm2_mode = MODE_IDLE;
            send_status(2);
            break;

        default:
        case PATH_NONE:
            break;
        }
    }
}


void MainWindow::enqueueArm1(PathElement P)
{
    arm1path.addElement(P);
}

void MainWindow::enqueueArm2(PathElement P)
{
    arm2path.addElement(P);
}

void MainWindow::settingChange1( int settingId, int settingVal )
{
    if( settingId == SETTING_MODE )
    {
        arm1_mode = static_cast<EspMode>(settingVal);
        arm2_mode = static_cast<EspMode>(settingVal);
    }
}

void MainWindow::settingChange2( int settingId, int settingVal )
{
    if( settingId == SETTING_MODE )
    {
        arm2_mode = static_cast<EspMode>(settingVal);
    }
}


static float parseFloatStrict( std::string str ) {
    size_t lenParsed;
    float result = std::stof(str, &lenParsed);

    if( lenParsed != str.length() ) throw std::invalid_argument(str + " is not a valid floating point value");
    return result;
}

void MainWindow::server1_newConnection()
{
    while( server1->hasPendingConnections() )
    {
        QTcpSocket *newConn = server1->nextPendingConnection();
        ReceiveWrapper *w = new ReceiveWrapper(newConn, &arm1Buffer, this);
        connect(w, &ReceiveWrapper::point_received, this, &MainWindow::enqueueArm1);
        connect(w, &ReceiveWrapper::setting_received, this, &MainWindow::settingChange1);
    }
}

void MainWindow::server2_newConnection()
{
    while( server2->hasPendingConnections() )
    {
        QTcpSocket *newConn = server2->nextPendingConnection();
        ReceiveWrapper *w = new ReceiveWrapper(newConn, &arm2Buffer, this);
        connect(w, &ReceiveWrapper::point_received, this, &MainWindow::enqueueArm2);
        connect(w, &ReceiveWrapper::setting_received, this, &MainWindow::settingChange2);
    }
}

void MainWindow::on_resetButton_clicked()
{
    ui->armCanvas->reset();

    arm1_mode = MODE_IDLE;
    arm1path.clear();

    arm2_mode = MODE_IDLE;
    arm2path.clear();
}
