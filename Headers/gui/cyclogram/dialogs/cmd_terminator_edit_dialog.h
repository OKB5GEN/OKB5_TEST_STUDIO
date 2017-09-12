#ifndef CMD_TERMINATOR_EDIT_DIALOG_H
#define CMD_TERMINATOR_EDIT_DIALOG_H

#include "Headers/gui/tools/restorable_dialog.h"

class CmdTitle;
class ConsoleTextWidget;

class CmdTerminatorEditDialog : public RestorableDialog
{
    Q_OBJECT

public:
    CmdTerminatorEditDialog(QWidget * parent);
    ~CmdTerminatorEditDialog();

    void setCommand(CmdTitle* command);

protected:

private slots:
    void onAccept();

private:
    CmdTitle* mCommand;
    ConsoleTextWidget* mConsoleTextWidget;
};

#endif // CMD_TERMINATOR_EDIT_DIALOG_H
