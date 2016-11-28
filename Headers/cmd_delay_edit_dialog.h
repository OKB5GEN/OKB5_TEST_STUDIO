#ifndef CMD_DELAY_EDIT_DIALOG_H
#define CMD_DELAY_EDIT_DIALOG_H

#include <QDialog>

class CmdDelay;

class CmdDelayEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdDelayEditDialog(QWidget * parent);
    ~CmdDelayEditDialog();

    void setCommand(CmdDelay* command);

private slots:

private:
};

#endif // CMD_DELAY_EDIT_DIALOG_H
