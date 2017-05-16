#ifndef CMD_OUTPUT_EDIT_DIALOG_H
#define CMD_OUTPUT_EDIT_DIALOG_H

#include <QDialog>

class Command;

class CmdOutputEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdOutputEditDialog(QWidget * parent);
    ~CmdOutputEditDialog();

    void setCommand(Command* command);

protected:

private slots:

private:
};

#endif // CMD_OUTPUT_EDIT_DIALOG_H
