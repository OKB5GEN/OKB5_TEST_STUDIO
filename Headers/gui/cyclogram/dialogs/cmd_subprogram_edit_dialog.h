#ifndef CMD_SUBPROGRAM_EDIT_DIALOG_H
#define CMD_SUBPROGRAM_EDIT_DIALOG_H

#include <QDialog>

class QComboBox;
class QCheckBox;
class QRadioButton;
class QLineEdit;
class QGroupBox;
class QDoubleValidator;
class QTableWidget;

class CmdSubProgram;
class Cyclogram;

class CmdSubProgramEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdSubProgramEditDialog(QWidget * parent);
    ~CmdSubProgramEditDialog();

    void setCommand(CmdSubProgram* command);
    void setCallingCyclogram(Cyclogram* cyclogram);

private slots:
    void onAccept();

//    void onCheckBoxStateChanged(int state);
//    void onOper1VarBtnStateChanged(bool toggled);
//    void onOper1NumBtnStateChanged(bool toggled);
//    void onOper2VarBtnStateChanged(bool toggled);
//    void onOper2NumBtnStateChanged(bool toggled);

    void openFile();

    void onCheckBoxStateChanged(int state);

private:
    void setupUI();
//    void updateComponent(int operand, QComboBox* box, QLineEdit* lineEdit, QRadioButton* boxBtn, QRadioButton* lineEditBtn);

    void updateUI();

    CmdSubProgram* mCommand;

    QLineEdit* mFileNameStr;
    QLineEdit* mSubprogramNameStr;

//    QComboBox* mResultBox;
//    QComboBox* mOper1Box;
//    QComboBox* mOperationBox;
//    QComboBox* mOper2Box;

//    QCheckBox* mTwoOperandsCheckBox;

//    QRadioButton* mOper1VarBtn;
//    QRadioButton* mOper1NumBtn;
//    QRadioButton* mOper2VarBtn;
//    QRadioButton* mOper2NumBtn;

//    QLineEdit* mOper1Num;
//    QLineEdit* mOper2Num;

//    QGroupBox* mOperand1Box;
//    QGroupBox* mOperand2Box;

//    QDoubleValidator* mValidator;

    QTableWidget* mInParams;
    QTableWidget* mOutParams;

    Cyclogram* mCallingCyclogram;
};

#endif // CMD_SUBPROGRAM_EDIT_DIALOG_H
