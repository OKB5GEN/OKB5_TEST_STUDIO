#ifndef SUBPROGRAM_DIALOG_H
#define SUBPROGRAM_DIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QTableWidget;
class QScrollArea;

class CmdSubProgram;
class CyclogramWidget;
class Cyclogram;
class VariablesWindow;

class SubProgramDialog : public QDialog
{
    Q_OBJECT

public:
    SubProgramDialog(CmdSubProgram* command, QWidget * parent);
    ~SubProgramDialog();

//    void setCommand(CmdSubProgram* command, Cyclogram* cyclogram);

private slots:
//    void onAccept();
//    void openFile();
//    void onInputCheckBoxStateChanged(int state);
//    void onOutputCheckBoxStateChanged(int state);
    void onSaveClick();
    void onVariablesClick();
    void onChartClick();

private:
//    void setupUI();
//    void updateUI();
//    void updateTable(QTableWidget* widget, QCheckBox* changedBox, int state);

    CmdSubProgram* mCommand;
    CyclogramWidget* mCyclogramWidget;
    VariablesWindow* mVariablesWindow;

    QScrollArea * mScrollArea;

//    Cyclogram* mCallingCyclogram;
};

#endif // SUBPROGRAM_DIALOG_H
