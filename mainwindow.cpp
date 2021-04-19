#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kinematics.h"
#include "pathiterator.h"
#include <QtMath>
#include <QFileDialog>
#include <fstream>
#include <sstream>


QHostAddress rpi_addr("127.0.0.1");

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
}

MainWindow::~MainWindow()
{
    delete ui;
    delete server1;
    delete server2;
}

void MainWindow::toggle_drawing( int settingMode, int settingValue )
{
    if(settingMode == SETTING_MODE)
    {
        if( settingValue == MODE_DRAW )
        {
            moveTimer.start();
            arm1_mode = MODE_DRAW;
            arm2_mode = MODE_DRAW;
        }
        else
        {
            moveTimer.stop();
            arm1_mode = MODE_PAUSE;
            arm2_mode = MODE_PAUSE;
        }
    }
}

void MainWindow::send_status( int device )
{
    if( device == 1 )
    {
        // pkt_type, id, power, mode, shoulder, elbow, odo, remaining
        QString data = QString("1,%1,1,%2,0,0,0,%3\n").arg(device, arm1_mode, arm1path.remaining());
        TransmitWrapper *w = new TransmitWrapper(this);
        w->start_transmit(rpi_addr, data.toUtf8());
    }
    else
    {

        QString data = QString("1,%1,1,%2,0,0,0,%3\n").arg(device, arm2_mode, arm2path.remaining());
        TransmitWrapper *w = new TransmitWrapper(this);
        w->start_transmit(rpi_addr, data.toUtf8());
    }
}


void MainWindow::moveTimer_timeout()
{
    struct arm_angles ang;
    PathElement nextMove = arm1path.moveNext();

    switch( nextMove.type )
    {
    case PATH_MOVE:
        ang = calculate_angles(nextMove.x, nextMove.y);
        ui->armCanvas->setArmPosition(ang, nextMove.x, nextMove.y);
        break;

    case PATH_PEN_UP:
        ui->armCanvas->penDown = false;
        break;

    case PATH_PEN_DOWN:
        ui->armCanvas->penDown = true;
        break;

    default:
        break;
    }

    // second arm
    nextMove = arm2path.moveNext();
    switch( nextMove.type )
    {
    case PATH_MOVE:
        ang = calculate_angles(nextMove.x, nextMove.y);
        ui->armCanvas->setArm2Position(ang, nextMove.x, nextMove.y);
        break;

    case PATH_PEN_UP:
        ui->armCanvas->penDown = false;
        break;

    case PATH_PEN_DOWN:
        ui->armCanvas->penDown = true;
        break;

    default:
        break;
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
        connect(w, &ReceiveWrapper::setting_received, this, &MainWindow::toggle_drawing);
    }
}

void MainWindow::server2_newConnection()
{
    while( server2->hasPendingConnections() )
    {
        QTcpSocket *newConn = server2->nextPendingConnection();
        ReceiveWrapper *w = new ReceiveWrapper(newConn, &arm2Buffer, this);
        connect(w, &ReceiveWrapper::point_received, this, &MainWindow::enqueueArm2);
        connect(w, &ReceiveWrapper::setting_received, this, &MainWindow::toggle_drawing);
    }
}
