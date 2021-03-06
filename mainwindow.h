#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pathiterator.h"
#include "piwifi.h"
#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QtNetwork/QTcpServer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum EspSetting
{
    SETTING_MODE = 1,
    SETTING_SPEED = 2
};

enum EspMode
{
    MODE_IDLE = 0,
    MODE_DRAW = 1,
    MODE_PAUSE = 2
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void enqueueArm1(PathElement P);
    void enqueueArm2(PathElement P);
    void settingChange1( int settingId, int settingVal );
    void settingChange2( int settingId, int settingVal );

private slots:
    void moveTimer_timeout();

    void server1_newConnection();
    void server2_newConnection();

    void on_resetButton_clicked();
    void on_singleArmCheck_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QTimer moveTimer;

    PathQueueIterator arm1path;
    PathQueueIterator arm2path;

    QTcpServer *server1;
    QTcpServer *server2;
    WPacketBuffer arm1Buffer;
    WPacketBuffer arm2Buffer;

    EspMode arm1_mode = MODE_IDLE;
    EspMode arm2_mode = MODE_IDLE;

    QTime lastStatus1 = QTime(0,0);
    QTime lastStatus2 = QTime(0,0);

    void send_status( int device );
};
#endif // MAINWINDOW_H
