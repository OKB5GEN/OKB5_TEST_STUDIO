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
    Sources/commands/cmd_cycle_do_while.cpp \
    Sources/commands/cmd_cycle_for.cpp \
    Sources/commands/cmd_cycle_while.cpp \
    Sources/commands/cmd_delay.cpp \
    Sources/commands/cmd_parallel_process.cpp \
    Sources/commands/cmd_send_msg_result_fail.cpp \
    Sources/commands/cmd_set_state.cpp \
    Sources/commands/cmd_state_start.cpp \
    Sources/commands/cmd_sub_program.cpp \
    Sources/commands/cmd_switch.cpp \
    Sources/monitor_auto.cpp \
    Sources/monitor_manual.cpp \
    Sources/commands/cmd_title.cpp \
    Sources/valency_point.cpp \
    Sources/cyclogram_end_dialog.cpp \
    Sources/cmd_delay_edit_dialog.cpp \
    Sources/cmd_state_start_edit_dialog.cpp \
    Sources/cmd_set_state_edit_dialog.cpp \
    Sources/shape_add_dialog.cpp \
    Sources/commands/cmd_action.cpp \
    Sources/cyclogram_widget.cpp \
    Sources/shape_item.cpp \
    Sources/editor_window.cpp \
    Sources/monitor_dialog.cpp \
    Sources/render_area.cpp \
    Sources/shape_edit_dialog.cpp \
    Sources/command_error_dialog.cpp \
    Sources/variable_controller.cpp \
    Sources/commands/cmd_question.cpp \
    Sources/commands/cmd_action_math.cpp \
    Sources/cmd_action_math_edit_dialog.cpp

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
    Headers/commands/cmd_cycle_do_while.h \
    Headers/commands/cmd_cycle_for.h \
    Headers/commands/cmd_cycle_while.h \
    Headers/commands/cmd_delay.h \
    Headers/commands/cmd_parallel_process.h \
    Headers/commands/cmd_send_msg_result_fail.h \
    Headers/commands/cmd_set_state.h \
    Headers/commands/cmd_state_start.h \
    Headers/commands/cmd_sub_program.h \
    Headers/commands/cmd_switch.h \
    Headers/command.h \
    Headers/monitor_auto.h \
    Headers/monitor_manual.h \
    Headers/commands/cmd_title.h \
    Headers/valency_point.h \
    Headers/cyclogram_end_dialog.h \
    Headers/cmd_delay_edit_dialog.h \
    Headers/cmd_state_start_edit_dialog.h \
    Headers/cmd_set_state_edit_dialog.h \
    Headers/shape_add_dialog.h \
    Headers/commands/cmd_action.h \
    Headers/cyclogram_widget.h \
    Headers/shape_item.h \
    Headers/editor_window.h \
    Headers/shape_types.h \
    Headers/monitor_dialog.h \
    Headers/render_area.h \
    Headers/shape_edit_dialog.h \
    Headers/command_error_dialog.h \
    Headers/variable_controller.h \
    Headers/commands/cmd_question.h \
    Headers/commands/cmd_action_math.h \
    Headers/cmd_action_math_edit_dialog.h


FORMS    += mainwindow.ui

RESOURCES += \
    application.qrc