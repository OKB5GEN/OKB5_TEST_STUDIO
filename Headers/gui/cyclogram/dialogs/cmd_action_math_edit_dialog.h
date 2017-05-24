#ifndef CMD_ACTION_MATH_EDIT_DIALOG_H
#define CMD_ACTION_MATH_EDIT_DIALOG_H

#include <QDialog>

class QComboBox;
class QCheckBox;
class QRadioButton;
class QLineEdit;
class QGroupBox;
class QDoubleValidator;

class CmdActionMath;
class ConsoleTextWidget;

class CmdActionMathEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdActionMathEditDialog(QWidget * parent);
    ~CmdActionMathEditDialog();

    void setCommand(CmdActionMath* command);

private slots:
    void onAccept();

    void onCheckBoxStateChanged(int state);
    void onOper1VarBtnStateChanged(bool toggled);
    void onOper1NumBtnStateChanged(bool toggled);
    void onOper2VarBtnStateChanged(bool toggled);
    void onOper2NumBtnStateChanged(bool toggled);

private:
    void setupUI();
    void updateComponent(int operand, QComboBox* box, QLineEdit* lineEdit, QRadioButton* boxBtn, QRadioButton* lineEditBtn);

    bool eventFilter(QObject *obj, QEvent *event) override;

    CmdActionMath* mCommand;

    QComboBox* mResultBox;
    QComboBox* mOper1Box;
    QComboBox* mOperationBox;
    QComboBox* mOper2Box;

    QCheckBox* mTwoOperandsCheckBox;

    QRadioButton* mOper1VarBtn;
    QRadioButton* mOper1NumBtn;
    QRadioButton* mOper2VarBtn;
    QRadioButton* mOper2NumBtn;

    QLineEdit* mOper1Num;
    QLineEdit* mOper2Num;

    QGroupBox* mOperand1Box;
    QGroupBox* mOperand2Box;

    QDoubleValidator* mValidator;

    ConsoleTextWidget* mConsoleTextWidget;
};

#endif // CMD_ACTION_MATH_EDIT_DIALOG_H
