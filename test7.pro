#-------------------------------------------------
#
# Project created by QtCreator 2016-08-04T09:53:43
#
#-------------------------------------------------

QT       += core gui

INCLUDEPATH += C:\Qt\5.3\mingw482_32\include
INCLUDEPATH +=C:\Qt\Tools\mingw482_32\i686-w64-mingw32\include

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = test7
TEMPLATE = app
QT += serialport
CONFIG +=c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    myclass.cpp \
    comport.cpp \
    OTD.cpp \
    MKO.cpp \
    Sources/command.cpp \
    Sources/commandscontroller.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    myclass.h \
    comport.h \
    OTD.h \
    WDMTMKv2.h \
    MKO.h \
    Headers/command.h \
    Headers/commandscontroller.h


FORMS    += mainwindow.ui
