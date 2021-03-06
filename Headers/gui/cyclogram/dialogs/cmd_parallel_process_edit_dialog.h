#ifndef CMD_PARALLEL_PROCESS_EDIT_DIALOG_H
#define CMD_PARALLEL_PROCESS_EDIT_DIALOG_H

#include "Headers/gui/tools/restorable_dialog.h"

class Command;

class CmdParallelProcessEditDialog : public RestorableDialog
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
