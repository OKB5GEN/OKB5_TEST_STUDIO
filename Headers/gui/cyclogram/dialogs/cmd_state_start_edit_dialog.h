#ifndef CMD_STATE_START_EDIT_DIALOG_H
#define CMD_STATE_START_EDIT_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

class CmdStateStart;
class Command;

class CmdStateStartEditDialog : public QDialog
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
};

#endif // CMD_STATE_START_EDIT_DIALOG_H
