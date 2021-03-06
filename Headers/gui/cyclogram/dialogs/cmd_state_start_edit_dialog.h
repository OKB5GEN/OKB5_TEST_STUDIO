#ifndef CMD_STATE_START_EDIT_DIALOG_H
#define CMD_STATE_START_EDIT_DIALOG_H

#include "Headers/gui/tools/restorable_dialog.h"

class QLineEdit;

class CmdStateStart;
class Command;
class ConsoleTextWidget;

class CmdStateStartEditDialog : public RestorableDialog
{
    Q_OBJECT

public:
    CmdStateStartEditDialog(QWidget * parent);
    ~CmdStateStartEditDialog();

    void setCommands(CmdStateStart* command, const QList<Command*>& otherBranches);

private slots:
    void onAccept();

private:
    QList<Command*> mOtherBranches;
    QLineEdit* mLineEdit;
    CmdStateStart* mCommand;
    ConsoleTextWidget* mConsoleTextWidget;
};

#endif // CMD_STATE_START_EDIT_DIALOG_H
