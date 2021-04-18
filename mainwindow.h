#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pathiterator.h"
#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_xSlider_valueChanged(int value);
    void on_ySlider_valueChanged(int value);
    void on_circleButton_clicked();
    void moveTimer_timeout();
    void on_squareButton_clicked();
    void on_clearButton_clicked();
    void on_gotoButton_clicked();
    void on_penUpButton_clicked();
    void on_penDownButton_clicked();

    void on_openPathButton_clicked();

private:
    Ui::MainWindow *ui;
    QTimer moveTimer;
    IPathIterator *path;

    bool drawing = false;
    void toggle_drawing( bool go );

    void recalc_angles( float x, float y );
};
#endif // MAINWINDOW_H
