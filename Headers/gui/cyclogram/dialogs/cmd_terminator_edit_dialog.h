#ifndef CMD_TERMINATOR_EDIT_DIALOG_H
#define CMD_TERMINATOR_EDIT_DIALOG_H

#include <QDialog>

class CmdTitle;
class ConsoleTextWidget;

class CmdTerminatorEditDialog : public QDialog
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
