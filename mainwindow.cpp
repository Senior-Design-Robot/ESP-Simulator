#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kinematics.h"
#include "pathiterator.h"
#include <QtMath>
#include <QFileDialog>
#include <fstream>
#include <sstream>

PathQueueIterator arm1path;
PathQueueIterator arm2path;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), path(nullptr)
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

void MainWindow::toggle_drawing( bool go )
{
    if( go ) moveTimer.start();
    else moveTimer.stop();

    drawing = go;
}

void MainWindow::recalc_angles( float x, float y )
{
    struct arm_angles ang = calculate_angles(x, y);
    ui->armCanvas->setArmPosition(ang, x, y);

    float s_ang = rad_to_deg(ang.shoulder);
    float e_ang = rad_to_deg(ang.elbow);

void MainWindow::moveTimer_timeout()
{
    if( !path )
    {
        moveTimer.stop();
        return;
    }

    PathElement nextMove = path->moveNext();
    switch( nextMove.type )
    {
    case PATH_MOVE:
        recalc_angles(nextMove.x, nextMove.y);
        break;

    case PATH_PEN_UP:
        ui->armCanvas->penDown = false;
        break;

    case PATH_PEN_DOWN:
        ui->armCanvas->penDown = true;
        break;

    default:
        moveTimer.stop();
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
        new ReceiveWrapper(newConn, &arm1Buffer, this);
    }
}

void MainWindow::server2_newConnection()
{
    while( server2->hasPendingConnections() )
    {
        QTcpSocket *newConn = server2->nextPendingConnection();
        new ReceiveWrapper(newConn, &arm2Buffer, this);
    }
}
