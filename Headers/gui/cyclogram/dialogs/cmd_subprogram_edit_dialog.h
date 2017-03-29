#ifndef CMD_SUBPROGRAM_EDIT_DIALOG_H
#define CMD_SUBPROGRAM_EDIT_DIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QTableWidget;

class CmdSubProgram;
class Cyclogram;

class CmdSubProgramEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdSubProgramEditDialog(QWidget * parent);
    ~CmdSubProgramEditDialog();

    void setCommand(CmdSubProgram* command, Cyclogram* cyclogram);

private slots:
    void onAccept();
    void openFile();
    void onInputCheckBoxStateChanged(int state);
    void onOutputCheckBoxStateChanged(int state);

private:
    void setupUI();
    void updateUI();
    void updateTable(QTableWidget* widget, QCheckBox* changedBox, int state);

    CmdSubProgram* mCommand;

    QLineEdit* mFileNameStr;
    QLineEdit* mSubprogramNameStr;

    QTableWidget* mInParams;
    QTableWidget* mOutParams;

    Cyclogram* mCallingCyclogram;
};

#endif // CMD_SUBPROGRAM_EDIT_DIALOG_H
