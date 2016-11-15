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
    Sources/cyclogram.cpp \
    Sources/cell.cpp \
    Sources/commands/cmd_case.cpp \
    Sources/commands/cmd_check_condition.cpp \
    Sources/commands/cmd_cycle_do_while.cpp \
    Sources/commands/cmd_cycle_for.cpp \
    Sources/commands/cmd_cycle_while.cpp \
    Sources/commands/cmd_delay.cpp \
    Sources/commands/cmd_parallel_process.cpp \
    Sources/commands/cmd_send_msg_ok_fail.cpp \
    Sources/commands/cmd_send_msg_result.cpp \
    Sources/commands/cmd_send_msg_result_fail.cpp \
    Sources/commands/cmd_set_state.cpp \
    Sources/commands/cmd_state_start.cpp \
    Sources/commands/cmd_sub_program.cpp \
    Sources/commands/cmd_switch.cpp \
    Sources/commands/cmd_variable.cpp \
    Sources/editorwindow.cpp \
    Sources/renderarea.cpp \
    Sources/shapeitem.cpp \
    Sources/sortingbox.cpp \
    Sources/shapeadddialog.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    myclass.h \
    comport.h \
    OTD.h \
    WDMTMKv2.h \
    MKO.h \
    Headers/cyclogram.h \
    Headers/cell.h \
    Headers/commands/cmd_case.h \
    Headers/commands/cmd_check_condition.h \
    Headers/commands/cmd_cycle_do_while.h \
    Headers/commands/cmd_cycle_for.h \
    Headers/commands/cmd_cycle_while.h \
    Headers/commands/cmd_delay.h \
    Headers/commands/cmd_parallel_process.h \
    Headers/commands/cmd_send_msg_ok_fail.h \
    Headers/commands/cmd_send_msg_result.h \
    Headers/commands/cmd_send_msg_result_fail.h \
    Headers/commands/cmd_set_state.h \
    Headers/commands/cmd_state_start.h \
    Headers/commands/cmd_sub_program.h \
    Headers/commands/cmd_switch.h \
    Headers/commands/cmd_variable.h \
    Headers/command.h \
    Headers/editorwindow.h \
    Headers/renderarea.h \
    Headers/shapeitem.h \
    Headers/sortingbox.h \
    Headers/shapeadddialog.h \
    Headers/shapetypes.h


FORMS    += mainwindow.ui

RESOURCES += \
    application.qrc
