#ifndef CMD_OUTPUT_EDIT_DIALOG_H
#define CMD_OUTPUT_EDIT_DIALOG_H

#include "Headers/gui/tools/restorable_dialog.h"

class Command;
class ConsoleTextWidget;

class CmdOutputEditDialog : public RestorableDialog
{
    Q_OBJECT

public:
    CmdOutputEditDialog(QWidget * parent);
    ~CmdOutputEditDialog();

    void setCommand(Command* command);

private slots:
    void onAccept();

private:
    Command* mCommand;
    ConsoleTextWidget* mConsoleTextWidget;
};

#endif // CMD_OUTPUT_EDIT_DIALOG_H
