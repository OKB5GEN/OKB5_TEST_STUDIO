#ifndef CMD_PARALLEL_PROCESS_EDIT_DIALOG_H
#define CMD_PARALLEL_PROCESS_EDIT_DIALOG_H

#include <QDialog>

class Command;

class CmdParallelProcessEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdParallelProcessEditDialog(QWidget * parent);
    ~CmdParallelProcessEditDialog();

    void setCommand(Command* command);

protected:

private slots:

private:
};

#endif // CMD_PARALLEL_PROCESS_EDIT_DIALOG_H
