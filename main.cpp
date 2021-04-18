#include "mainwindow.h"
#include "queuedialog.h"
#include "dynamixeldialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    //DynamixelDialog dd;
    //dd.show();

    return a.exec();
}
