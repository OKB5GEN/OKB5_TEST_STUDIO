#-------------------------------------------------
#
# Project created by QtCreator 2016-08-04T09:53:43
#
#-------------------------------------------------

QT       += core gui

INCLUDEPATH += C:\Qt\5.3\mingw482_32\include
INCLUDEPATH +=C:\Qt\Tools\mingw482_32\i686-w64-mingw32\include

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

include(external/qtxlsx/src/xlsx/qtxlsx.pri)

TARGET = OKB5TestStudio
TEMPLATE = app
QT += serialport
CONFIG +=c++11

#DEFINES += ENABLE_CYCLOGRAM_PAUSE

SOURCES += main.cpp\
    Sources/gui/cyclogram/dialogs/cmd_action_math_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cmd_delay_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cmd_question_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cmd_set_state_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cmd_state_start_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/command_error_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cyclogram_end_dialog.cpp \
    Sources/gui/cyclogram/dialogs/shape_add_dialog.cpp \
    Sources/gui/cyclogram/cyclogram_widget.cpp \
    Sources/gui/cyclogram/shape_item.cpp \
    Sources/gui/cyclogram/valency_point.cpp \
    Sources/gui/cyclogram/variables_window.cpp \
    Sources/gui/editor_window.cpp \
    Sources/logic/commands/cmd_action.cpp \
    Sources/logic/commands/cmd_action_math.cpp \
    Sources/logic/commands/cmd_case.cpp \
    Sources/logic/commands/cmd_delay.cpp \
    Sources/logic/commands/cmd_parallel_process.cpp \
    Sources/logic/commands/cmd_question.cpp \
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
    Sources/file_reader.cpp \
    Sources/logger/AbstractAppender.cpp \
    Sources/logger/AbstractStringAppender.cpp \
    Sources/logger/ConsoleAppender.cpp \
    Sources/logger/FileAppender.cpp \
    Sources/logger/Logger.cpp \
    Sources/logger/OutputDebugAppender.cpp \
    Sources/logger/RollingFileAppender.cpp \
    Sources/system/abstract_module.cpp \
    Sources/gui/cyclogram/dialogs/cmd_subprogram_edit_dialog.cpp \
    Sources/system/codecs/mko_codec.cpp \
    Sources/system/codecs/okb_codec.cpp \
    Sources/system/codecs/power_unit_codec.cpp \
    Sources/logger/TextEditAppender.cpp \
    Sources/gui/tools/app_console.cpp \
    Sources/gui/cyclogram/dialogs/subprogram_dialog.cpp \
    Sources/logic/cyclogram_manager.cpp \
    Sources/gui/modal_cyclogram_execution_dialog.cpp \
    Sources/gui/tools/cyclogram_console.cpp \
    Sources/gui/tools/console_text_widget.cpp \
    Sources/gui/cyclogram/dialogs/cmd_terminator_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cmd_output_edit_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cmd_parallel_process_edit_dialog.cpp \
    Sources/logic/commands/cmd_output.cpp \
    Sources/system/modules/module_drive_simulator.cpp \
    Sources/gui/tools/cyclogram_chart_dialog.cpp \
    Sources/gui/cyclogram/dialogs/cyclogram_settings_dialog.cpp \
    Sources/app_settings.cpp \
    Sources/gui/tools/app_settings_dialog.cpp \
    Sources/gui/cyclogram/dialogs/text_edit_dialog.cpp

HEADERS  += Headers/shape_types.h \
    Headers/gui/editor_window.h \
    Headers/gui/cyclogram/cyclogram_widget.h \
    Headers/gui/cyclogram/shape_item.h \
    Headers/gui/cyclogram/valency_point.h \
    Headers/gui/cyclogram/variables_window.h \
    Headers/gui/cyclogram/dialogs/cmd_action_math_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/cmd_delay_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/cmd_question_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/cmd_set_state_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/cmd_state_start_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/command_error_dialog.h \
    Headers/gui/cyclogram/dialogs/cyclogram_end_dialog.h \
    Headers/gui/cyclogram/dialogs/shape_add_dialog.h \
    Headers/logic/command.h \
    Headers/logic/cyclogram.h \
    Headers/logic/variable_controller.h \
    Headers/logic/commands/cmd_action.h \
    Headers/logic/commands/cmd_action_math.h \
    Headers/logic/commands/cmd_case.h \
    Headers/logic/commands/cmd_delay.h \
    Headers/logic/commands/cmd_parallel_process.h \
    Headers/logic/commands/cmd_question.h \
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
    Headers/file_reader.h \
    Headers/logger/AbstractAppender.h \
    Headers/logger/AbstractStringAppender.h \
    Headers/logger/ConsoleAppender.h \
    Headers/logger/CuteLogger_global.h \
    Headers/logger/FileAppender.h \
    Headers/logger/Logger.h \
    Headers/logger/OutputDebugAppender.h \
    Headers/logger/RollingFileAppender.h \
    Headers/system/abstract_module.h \
    Headers/gui/cyclogram/dialogs/cmd_subprogram_edit_dialog.h \
    Headers/system/codecs/mko_codec.h \
    Headers/system/codecs/okb_codec.h \
    Headers/system/codecs/power_unit_codec.h \
    Headers/logger/TextEditAppender.h \
    Headers/gui/tools/app_console.h \
    Headers/gui/cyclogram/dialogs/subprogram_dialog.h \
    Headers/logic/cyclogram_manager.h \
    Headers/gui/modal_cyclogram_execution_dialog.h \
    Headers/gui/tools/cyclogram_console.h \
    Headers/gui/tools/console_text_widget.h \
    Headers/gui/cyclogram/dialogs/cmd_terminator_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/cmd_output_edit_dialog.h \
    Headers/gui/cyclogram/dialogs/cmd_parallel_process_edit_dialog.h \
    Headers/logic/commands/cmd_output.h \
    Headers/system/modules/module_drive_simulator.h \
    Headers/gui/tools/cyclogram_chart_dialog.h \
    Headers/gui/cyclogram/dialogs/cyclogram_settings_dialog.h \
    Headers/app_settings.h \
    Headers/gui/tools/app_settings_dialog.h \
    Headers/gui/cyclogram/dialogs/text_edit_dialog.h

TRANSLATIONS += OKB5TestStudio_ru.ts

FORMS    +=

RESOURCES += \
    application.qrc
