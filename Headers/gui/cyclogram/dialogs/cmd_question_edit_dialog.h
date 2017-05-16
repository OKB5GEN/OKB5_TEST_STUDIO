#ifndef CMD_QUESTION_EDIT_DIALOG_H
#define CMD_QUESTION_EDIT_DIALOG_H

#include <QDialog>

class QComboBox;
class QCheckBox;
class QRadioButton;
class QLineEdit;
class QGroupBox;
class QDoubleValidator;

class CmdQuestion;
class ConsoleTextWidget;

class CmdQuestionEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdQuestionEditDialog(QWidget * parent);
    ~CmdQuestionEditDialog();

    void setCommand(CmdQuestion* command);

private slots:
    void onAccept();

    void onOper1VarBtnStateChanged(bool toggled);
    void onOper1NumBtnStateChanged(bool toggled);
    void onOper2VarBtnStateChanged(bool toggled);
    void onOper2NumBtnStateChanged(bool toggled);
    void onYesDownBtnStateChanged(bool toggled);
    void onYesRightBtnStateChanged(bool toggled);

private:
    void setupUI();
    void updateComponent(int operand, QComboBox* box, QLineEdit* lineEdit, QRadioButton* boxBtn, QRadioButton* lineEditBtn);

    CmdQuestion* mCommand;

    QComboBox* mOper1Box;
    QComboBox* mOperationBox;
    QComboBox* mOper2Box;

    QRadioButton* mOper1VarBtn;
    QRadioButton* mOper1NumBtn;
    QRadioButton* mOper2VarBtn;
    QRadioButton* mOper2NumBtn;
    QRadioButton* mYesDownBtn;
    QRadioButton* mYesRightBtn;

    QLineEdit* mOper1Num;
    QLineEdit* mOper2Num;

    QGroupBox* mOperand1Box;
    QGroupBox* mOperand2Box;

    QDoubleValidator* mValidator;

    ConsoleTextWidget* mConsoleTextWidget;
};

#endif // CMD_QUESTION_EDIT_DIALOG_H
