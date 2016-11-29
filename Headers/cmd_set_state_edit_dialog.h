#ifndef CMD_SET_STATE_EDIT_DIALOG_H
#define CMD_SET_STATE_EDIT_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QComboBox;
QT_END_NAMESPACE

class CmdSetState;
class Command;

class CmdSetStateEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdSetStateEditDialog(QWidget * parent);
    ~CmdSetStateEditDialog();

    void setCommands(CmdSetState* command, const QList<Command*>& branches);

private slots:
    void onAccept();

private:
    int mCurrentIndex;
    QComboBox* mComboBox;
    CmdSetState* mCommand;
    QList<Command*> mBranches;
};

#endif // CMD_SET_STATE_EDIT_DIALOG_H
