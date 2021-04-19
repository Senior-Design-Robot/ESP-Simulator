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
    connect(&moveTimer, &QTimer::timeout, this, &MainWindow::moveTimer_timeout);
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

    ui->shoulderAng->setText(QString::number(ang.shoulder, 'f', 3));
    ui->elbowAng->setText(QString::number(ang.elbow, 'f', 3));
}

void MainWindow::on_xSlider_valueChanged(int value)
{
    if( drawing ) toggle_drawing(false);
    recalc_angles(value, ui->ySlider->value());
}

void MainWindow::on_ySlider_valueChanged(int value)
{
    if( drawing ) toggle_drawing(false);
    recalc_angles(ui->xSlider->value(), value);
}

void MainWindow::on_circleButton_clicked()
{
    delete path;

    path = new CirclePathIterator(ui->xSlider->value(), ui->ySlider->value(), ui->radiusSlider->value());
    toggle_drawing(true);
}

void MainWindow::on_squareButton_clicked()
{
    delete path;

    path = new SquarePathIterator(ui->xSlider->value(), ui->ySlider->value(), ui->radiusSlider->value());
    toggle_drawing(true);
}

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

void MainWindow::on_clearButton_clicked()
{
    ui->armCanvas->reset();
}

void MainWindow::on_stopButton_clicked()
{

}

void MainWindow::on_gotoButton_clicked()
{
    PathQueueIterator *pqi = dynamic_cast<PathQueueIterator*>(path);
    if( !pqi )
    {
        delete path;
        pqi = new PathQueueIterator();
    }

    float x = ui->gotoXSpin->value();
    float y = ui->gotoYSpin->value();

    //QPointF lastPoint = (ui->armCanvas->pastPoints.empty()) ? QPointF(0, 0) : ui->armCanvas->pastPoints.back();

    pqi->addMove(x, y);
    path = pqi;
    toggle_drawing(true);
}

void MainWindow::on_penUpButton_clicked()
{
    ui->armCanvas->penDown = false;
}

void MainWindow::on_penDownButton_clicked()
{
    ui->armCanvas->penDown = true;
}

static float parseFloatStrict( std::string str ) {
    size_t lenParsed;
    float result = std::stof(str, &lenParsed);

    if( lenParsed != str.length() ) throw std::invalid_argument(str + " is not a valid floating point value");
    return result;
}

void MainWindow::on_openPathButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Path", QFileInfo().absolutePath(), "Path Files (*.csv)");
    std::ifstream layoutFile;

    QFileInfo lfInfo(fileName);
    layoutFile.open(lfInfo.absoluteFilePath().toStdString(), std::ios::in);

    if( !layoutFile.is_open() ) throw std::invalid_argument ("Unable to open layout file");
    std::string nextLine;
    std::stringstream buf = std::stringstream ();

    // oh boy a parser
    enum ParseState {
        LL_X, LL_Y
    };

    PathQueueIterator *pqi = dynamic_cast<PathQueueIterator*>(path);
    if( !pqi )
    {
        delete path;
        pqi = new PathQueueIterator();
        path = pqi;
    }

    pqi->clear();
    pqi->addPenMove(false);

    ParseState state;
    int fileLine = 1;

    float ptX, ptY;
    bool firstPt = true;

    while( getline(layoutFile, nextLine) )
    {
        // check if line is commented
        if( (nextLine.length() == 0) || (nextLine.at(0) == '#') )
        {
            fileLine += 1;
            continue;
        }

        if( nextLine.at(0) == 'u' )
        {
            pqi->addPenMove(false);
            continue;
        }
        if( nextLine.at(0) == 'd' )
        {
            pqi->addPenMove(true);
            continue;
        }
        if( nextLine.at(0) == 'j' )
        {
            // jump to new segment
            pqi->addPenMove(false);
            firstPt = true;
            continue;
        }

        state = LL_X;
        auto iter = nextLine.begin();

        while( iter != nextLine.end() )
        {
            char nextChar = *iter;
            if( nextChar == ',' )
            {
                std::string bufStr = buf.str();

                if( state == LL_X )
                {
                    ptX = parseFloatStrict(bufStr);
                }
                else
                {
                    buf.str(std::string());
                    buf.clear();
                    buf << "Too many fields on line " << fileLine;
                    layoutFile.close();
                    throw std::invalid_argument(buf.str());
                }

                state = static_cast<ParseState>(state + 1);
                buf.str(std::string());
                buf.clear();
            }
            else
            {
                buf.put(nextChar);
            }

            iter++;
        }

        if( state == LL_Y )
        {
            std::string bufStr = buf.str();
            ptY = parseFloatStrict(bufStr);
            buf.str(std::string());
            buf.clear();
        }
        else
        {
            buf.str(std::string());
            buf.clear();
            buf << "Too few fields on line " << fileLine;
            layoutFile.close();
            throw std::invalid_argument(buf.str());
        }

        pqi->addMove(ptX, ptY);
        if( firstPt )
        {
            pqi->addPenMove(true);
            firstPt = false;
        }

        // proceed to next line
        fileLine += 1;
    }
    // end while( getline() )

    // eof
    layoutFile.close();

    pqi->addPenMove(false);
    pqi->addMove(-10, 0);

    toggle_drawing(true);
}
