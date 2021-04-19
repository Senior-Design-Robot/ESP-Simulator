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

public slots:
    void enqueueArm1(PathElement P);
    void enqueueArm2(PathElement P);

private slots:
    //void on_clearButton_clicked();

private:
    Ui::MainWindow *ui;
    QTimer moveTimer;
    IPathIterator *path;

    bool drawing = false;
    void toggle_drawing( bool go );

    void recalc_angles( float x, float y );
};
#endif // MAINWINDOW_H
