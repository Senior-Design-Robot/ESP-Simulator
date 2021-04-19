#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kinematics.h"
#include "pathiterator.h"
#include <QtMath>
#include <QFileDialog>
#include <fstream>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), path(nullptr)
{
    ui->setupUi(this);
    moveTimer.setInterval(10);
}

MainWindow::~MainWindow()
{
    delete ui;
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

}

/*void MainWindow::on_clearButton_clicked()
{
    ui->armCanvas->reset();
}*/

static float parseFloatStrict( std::string str ) {
    size_t lenParsed;
    float result = std::stof(str, &lenParsed);

    if( lenParsed != str.length() ) throw std::invalid_argument(str + " is not a valid floating point value");
    return result;
}
