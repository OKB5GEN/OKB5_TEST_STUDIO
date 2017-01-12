#-------------------------------------------------
#
# Project created by QtCreator 2016-08-04T09:53:43
#
#-------------------------------------------------

QT       += core gui

INCLUDEPATH += C:\Qt\5.3\mingw482_32\include
INCLUDEPATH +=C:\Qt\Tools\mingw482_32\i686-w64-mingw32\include

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = OKB5TestStudio
TEMPLATE = app
QT += serialport
CONFIG +=c++11

SOURCES += main.cpp\
    Sources/gui/cyclogram/dialogs/cmd_action_math_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cmd_delay_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cmd_question_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cmd_set_state_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cmd_state_start_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/command_error_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cyclogram_end_dialog.cpp \
    Sources/gui/cyclogram/dialogs/shape_add_dialog.cpp \
    Sources/gui/cyclogram/dialogs/shape_edit_dialog.cpp \
    Sources/gui/cyclogram/cyclogram_widget.cpp \
    Sources/gui/cyclogram/shape_item.cpp \
    Sources/gui/cyclogram/valency_point.cpp \
    Sources/gui/cyclogram/variables_window.cpp \
    Sources/gui/tools/monitor_auto.cpp \
    Sources/gui/tools/monitor_manual.cpp \
    Sources/gui/editor_window.cpp \
    Sources/logic/commands/cmd_action.cpp \
    Sources/logic/commands/cmd_action_math.cpp \
    Sources/logic/commands/cmd_case.cpp \
    Sources/logic/commands/cmd_cycle_do_while.cpp \
    Sources/logic/commands/cmd_cycle_for.cpp \
    Sources/logic/commands/cmd_cycle_while.cpp \
    Sources/logic/commands/cmd_delay.cpp \
    Sources/logic/commands/cmd_parallel_process.cpp \
    Sources/logic/commands/cmd_question.cpp \
    Sources/logic/commands/cmd_send_msg_result_fail.cpp \
    Sources/logic/commands/cmd_set_state.cpp \
    Sources/logic/commands/cmd_state_start.cpp \
    Sources/logic/commands/cmd_sub_program.cpp \
    Sources/logic/commands/cmd_switch.cpp \
    Sources/logic/commands/cmd_title.cpp \
    Sources/logic/command.cpp \
    Sources/logic/cyclogram.cpp \
    Sources/logic/variable_controller.cpp \
    Sources/gui/qcustomplot.cpp \
    Sources/system/WDMTMKv2.cpp \
    Sources/system/system_state.cpp \
    Sources/system/modules/module_mko.cpp \
    Sources/system/modules/module_otd.cpp \
    Sources/system/modules/module_stm.cpp \
    Sources/system/modules/module_tech.cpp \
    Sources/system/modules/module_power.cpp \
    Sources/system/com_port_module.cpp \
    Sources/system/okb_module.cpp \
    Sources/logic/commands/cmd_action_module.cpp \
    Sources/gui/cyclogram/dialogs/cmd_action_module_edit_dialog.cpp \
    Sources/file_writer.cpp \
    Sources/file_reader.cpp

HEADERS  += Headers/shape_types.h \
    Headers/gui/editor_window.h \
    Headers/gui/cyclogram/cyclogram_widget.h \
    Headers/gui/cyclogram/shape_item.h \
    Headers/gui/cyclogram/valency_point.h \
    Headers/gui/cyclogram/variables_window.h \
    Headers/gui/tools/monitor_auto.h \
    Headers/gui/tools/monitor_manual.h \
    Headers/gui/cyclogram/dialogs/cmd_action_math_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/cmd_delay_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/cmd_question_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/cmd_set_state_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/cmd_state_start_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/command_error_dialog.h \
    Headers/gui/cyclogram/dialogs/cyclogram_end_dialog.h \
    Headers/gui/cyclogram/dialogs/shape_add_dialog.h \
    Headers/gui/cyclogram/dialogs/shape_edit_dialog.h \
    Headers/logic/command.h \
    Headers/logic/cyclogram.h \
    Headers/logic/variable_controller.h \
    Headers/logic/commands/cmd_action.h \
    Headers/logic/commands/cmd_action_math.h \
    Headers/logic/commands/cmd_case.h \
    Headers/logic/commands/cmd_cycle_do_while.h \
    Headers/logic/commands/cmd_cycle_for.h \
    Headers/logic/commands/cmd_cycle_while.h \
    Headers/logic/commands/cmd_delay.h \
    Headers/logic/commands/cmd_parallel_process.h \
    Headers/logic/commands/cmd_question.h \
    Headers/logic/commands/cmd_send_msg_result_fail.h \
    Headers/logic/commands/cmd_set_state.h \
    Headers/logic/commands/cmd_state_start.h \
    Headers/logic/commands/cmd_sub_program.h \
    Headers/logic/commands/cmd_switch.h \
    Headers/logic/commands/cmd_title.h \
    Headers/gui/qcustomplot.h \
    Headers/system/WDMTMKv2.h \
    Headers/system/system_state.h \
    Headers/system/modules/module_mko.h \
    Headers/system/modules/module_otd.h \
    Headers/system/modules/module_stm.h \
    Headers/system/modules/module_tech.h \
    Headers/module_commands.h \
    Headers/system/modules/module_power.h \
    Headers/system/com_port_module.h \
    Headers/system/okb_module.h \
    Headers/logic/commands/cmd_action_module.h \
    Headers/gui/cyclogram/dialogs/cmd_action_module_edit_dialog.h \
    Headers/file_writer.h \
    Headers/file_reader.h


FORMS    += mainwindow.ui

RESOURCES += \
    application.qrc
