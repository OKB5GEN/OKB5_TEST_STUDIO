#ifndef CMD_SUBPROGRAM_EDIT_DIALOG_H
#define CMD_SUBPROGRAM_EDIT_DIALOG_H

#include <QDialog>
#include <QSharedPointer>

class QLineEdit;
class QCheckBox;
class QTableWidget;

class CmdSubProgram;
class Cyclogram;
class ConsoleTextWidget;

class CmdSubProgramEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdSubProgramEditDialog(QWidget * parent);
    ~CmdSubProgramEditDialog();

    void setCommand(CmdSubProgram* command, QSharedPointer<Cyclogram> cyclogram);

private slots:
    void onAccept();
    void openFile();
    void onInputCheckBoxStateChanged(int state);
    void onOutputCheckBoxStateChanged(int state);

private:
    void setupUI();
    void updateUI();
    void updateTable(QTableWidget* widget, QCheckBox* changedBox, int state);
    bool eventFilter(QObject *obj, QEvent *event) override;

    CmdSubProgram* mCommand;

    QLineEdit* mFileNameStr;
    QLineEdit* mSubprogramNameStr;

    QTableWidget* mInParams;
    QTableWidget* mOutParams;

    ConsoleTextWidget* mConsoleTextWidget;

    QWeakPointer<Cyclogram> mCallingCyclogram;
};

#endif // CMD_SUBPROGRAM_EDIT_DIALOG_H
