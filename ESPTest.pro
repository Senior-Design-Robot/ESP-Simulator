QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    armdisplay.cpp \
    dynamixeldialog.cpp \
    kinematics.cpp \
    main.cpp \
    mainwindow.cpp \
    pathiterator.cpp \
    piwifi.cpp \
    queuedialog.cpp

HEADERS += \
    armdisplay.h \
    dynamixel.h \
    dynamixeldialog.h \
    kinematics.h \
    kqueue.h \
    mainwindow.h \
    pathiterator.h \
    piwifi.h \
    queuedialog.h

FORMS += \
    dynamixeldialog.ui \
    mainwindow.ui \
    queuedialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
