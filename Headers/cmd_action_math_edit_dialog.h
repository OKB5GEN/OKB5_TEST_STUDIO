#ifndef CMD_ACTION_MATH_EDIT_DIALOG_H
#define CMD_ACTION_MATH_EDIT_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
class QRadioButton;
class QLineEdit;
class QGroupBox;
QT_END_NAMESPACE


class CmdActionMath;

class CmdActionMathEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdActionMathEditDialog(QWidget * parent);
    ~CmdActionMathEditDialog();

    void setCommand(CmdActionMath* command);

private slots:
    void onAccept();

    ///
    /// \brief onCheckBoxStateChanged
    /// \param state
    ///
    void onCheckBoxStateChanged(int state);
    void onOper1VarBtnStateChanged(bool toggled);
    void onOper1NumBtnStateChanged(bool toggled);
    void onOper2VarBtnStateChanged(bool toggled);
    void onOper2NumBtnStateChanged(bool toggled);

private:
    void setupUI();

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
};

#endif // CMD_ACTION_MATH_EDIT_DIALOG_H
