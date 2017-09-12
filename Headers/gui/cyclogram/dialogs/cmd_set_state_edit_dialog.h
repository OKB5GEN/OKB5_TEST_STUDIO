#ifndef CMD_SET_STATE_EDIT_DIALOG_H
#define CMD_SET_STATE_EDIT_DIALOG_H

#include "Headers/gui/tools/restorable_dialog.h"

class QListWidget;

class CmdSetState;
class Command;
class ConsoleTextWidget;

class CmdSetStateEditDialog : public RestorableDialog
{
    Q_OBJECT

public:
    CmdSetStateEditDialog(QWidget * parent);
    ~CmdSetStateEditDialog();

    void setCommands(CmdSetState* command, const QList<Command*>& branches);

private slots:
    void onAccept();

private:
    //bool eventFilter(QObject *obj, QEvent *event) override;

    int mCurrentIndex;
    QListWidget* mBranchesList;
    CmdSetState* mCommand;
    QList<Command*> mBranches;
    ConsoleTextWidget* mConsoleTextWidget;
};

#endif // CMD_SET_STATE_EDIT_DIALOG_H
