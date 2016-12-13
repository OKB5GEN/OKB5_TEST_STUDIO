#include <QtWidgets>

#include "Headers/gui/cyclogram/dialogs/cmd_action_math_edit_dialog.h"
#include "Headers/logic/commands/cmd_action_math.h"
#include "Headers/logic/variable_controller.h"

CmdActionMathEditDialog::CmdActionMathEditDialog(QWidget * parent):
    QDialog(parent),
    mCommand(Q_NULLPTR)
{
    setupUI();
    setWindowTitle(tr("Math operation"));

    adjustSize();
    setFixedSize(sizeHint());
}

CmdActionMathEditDialog::~CmdActionMathEditDialog()
{

}

void CmdActionMathEditDialog::setupUI()
{
    QGridLayout * layout = new QGridLayout(this);

    // Result box >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    QGroupBox* resultBox = new QGroupBox(this);
    resultBox->setTitle(tr("Result"));
    QVBoxLayout* box4layout = new QVBoxLayout(resultBox);
    mResultBox = new QComboBox(this);

    box4layout->addWidget(mResultBox);
    box4layout->addStretch();

    resultBox->setLayout(box4layout);
    layout->addWidget(resultBox, 0, 0, 2, 2);

    QLabel* equalSign = new QLabel(this);
    equalSign->setText("=");
    layout->addWidget(equalSign, 0, 2, 2, 1);

    mValidator = new QDoubleValidator(this);

    // Operand 1 box >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    mOperand1Box = new QGroupBox(this);
    mOperand1Box->setTitle(tr("Operand 1"));
    QGridLayout* box1layout = new QGridLayout(mOperand1Box);

    mOper1VarBtn = new QRadioButton(this);
    mOper1NumBtn = new QRadioButton(this);
    mOper1Box = new QComboBox(this);
    mOper1Num = new QLineEdit(this);
    mOper1Num->setValidator(mValidator);

    box1layout->addWidget(mOper1VarBtn, 0, 0, 1, 1);
    box1layout->addWidget(mOper1NumBtn, 1, 0, 1, 1);
    box1layout->addWidget(mOper1Box, 0, 1, 1, 1);
    box1layout->addWidget(mOper1Num, 1, 1, 1, 1);

    mOperand1Box->setLayout(box1layout);
    layout->addWidget(mOperand1Box, 0, 3, 2, 2);

    // Operation box >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    QGroupBox* operationBox = new QGroupBox(this);
    operationBox->setTitle(tr("Operation"));
    QVBoxLayout* box3layout = new QVBoxLayout(operationBox);
    mOperationBox = new QComboBox(this);
    mOperationBox->addItem("+", QVariant(int(CmdActionMath::Add)));
    mOperationBox->addItem("-", QVariant(int(CmdActionMath::Subtract)));
    mOperationBox->addItem("*", QVariant(int(CmdActionMath::Multiply)));
    mOperationBox->addItem(":", QVariant(int(CmdActionMath::Divide)));

    mTwoOperandsCheckBox = new QCheckBox(this);
    mTwoOperandsCheckBox->setText(tr("Two operands"));

    box3layout->addWidget(mOperationBox);
    box3layout->addWidget(mTwoOperandsCheckBox);

    operationBox->setLayout(box3layout);
    layout->addWidget(operationBox, 0, 5, 2, 2);

    // Operand 2 box >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    mOperand2Box = new QGroupBox(this);
    mOperand2Box->setTitle(tr("Operand 2"));
    QGridLayout* box2layout = new QGridLayout(mOperand2Box);

    mOper2VarBtn = new QRadioButton(this);
    mOper2NumBtn = new QRadioButton(this);
    mOper2Box = new QComboBox(this);
    mOper2Num = new QLineEdit(this);
    mOper2Num->setValidator(mValidator);

    box2layout->addWidget(mOper2VarBtn, 0, 0, 1, 1);
    box2layout->addWidget(mOper2NumBtn, 1, 0, 1, 1);
    box2layout->addWidget(mOper2Box, 0, 1, 1, 1);
    box2layout->addWidget(mOper2Num, 1, 1, 1, 1);

    mOperand2Box->setLayout(box2layout);

    layout->addWidget(mOperand2Box, 0, 7, 2, 2);

    // Dialog button box >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 3, 7, 1, 2);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(layout);

    connect(mTwoOperandsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxStateChanged(int)));
    connect(mOper1VarBtn, SIGNAL(toggled(bool)), this, SLOT(onOper1VarBtnStateChanged(bool)));
    connect(mOper1NumBtn, SIGNAL(toggled(bool)), this, SLOT(onOper1NumBtnStateChanged(bool)));
    connect(mOper2VarBtn, SIGNAL(toggled(bool)), this, SLOT(onOper2VarBtnStateChanged(bool)));
    connect(mOper2NumBtn, SIGNAL(toggled(bool)), this, SLOT(onOper2NumBtnStateChanged(bool)));
}

void CmdActionMathEditDialog::setCommand(CmdActionMath* command)
{
    mCommand = command;

    if (mCommand)
    {
        // set default state
        mTwoOperandsCheckBox->setChecked(true);
        mOper1VarBtn->setChecked(true);
        mOper2VarBtn->setChecked(true);
        mOper1Num->setText("0");
        mOper2Num->setText("0");

        mOper1Box->clear();
        mOper2Box->clear();
        mResultBox->clear();

        VariableController* vc = mCommand->variableController();
        mOper1Box->addItems(vc->variables().keys());
        mOper2Box->addItems(vc->variables().keys());
        mResultBox->addItems(vc->variables().keys());

        int index = mOperationBox->findData(QVariant(int(mCommand->operation())));
        if (index != -1)
        {
            mOperationBox->setCurrentIndex(index);
        }

        updateComponent(CmdActionMath::Result, mResultBox, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);
        updateComponent(CmdActionMath::Operand1, mOper1Box, mOper1Num, mOper1VarBtn, mOper1NumBtn);
        updateComponent(CmdActionMath::Operand2, mOper2Box, mOper2Num, mOper2VarBtn, mOper2NumBtn);
    }
}

void CmdActionMathEditDialog::updateComponent(int operand, QComboBox* box, QLineEdit* lineEdit, QRadioButton* boxBtn, QRadioButton* lineEditBtn)
{
    VariableController* vc = mCommand->variableController();
    CmdActionMath::OperandID op = CmdActionMath::OperandID(operand);

    if (mCommand->operandType(op) == CmdActionMath::Variable)
    {
        if (boxBtn)
        {
            boxBtn->setChecked(true);
        }

        QString name = mCommand->variableName(op);
        if (vc->isVariableExist(name))
        {
            int index = box->findText(name);
            if (index != -1)
            {
                box->setCurrentIndex(index);
            }
        }
    }
    else if (mCommand->operandType(op) == CmdActionMath::Number)
    {
        if (lineEditBtn)
        {
            lineEditBtn->setChecked(true);
        }

        if (lineEdit)
        {
            lineEdit->setText(QString::number(mCommand->value(op)));
        }
    }

    if (op == CmdActionMath::Operand2)
    {
        mTwoOperandsCheckBox->setChecked(mCommand->operation() != CmdActionMath::Assign);
    }
}

void CmdActionMathEditDialog::onAccept()
{
    if (mCommand)
    {
        if (mTwoOperandsCheckBox->isChecked())
        {
            CmdActionMath::Operation operation = CmdActionMath::Operation(mOperationBox->currentData().toInt());

            if (mOper2VarBtn->isChecked())
            {
                QString oper2Var = mOper2Box->currentText();
                mCommand->setOperand(CmdActionMath::Operand2, oper2Var);
            }
            else
            {
                qreal oper2Val = mOper2Num->text().replace(",", ".").toDouble();

                // division by zero protection
                if (operation == CmdActionMath::Divide && oper2Val == 0)
                {
                    QMessageBox::warning(this, tr("Error"), tr("Division by zero detected!"));
                    return;
                }

                mCommand->setOperand(CmdActionMath::Operand2, oper2Val);
            }

            mCommand->setOperation(operation);
        }
        else
        {
            mCommand->setOperation(CmdActionMath::Assign);
        }

        QString resultVar = mResultBox->currentText();
        mCommand->setOperand(CmdActionMath::Result, resultVar);

        if (mOper1VarBtn->isChecked())
        {
            QString oper1Var = mOper1Box->currentText();
            mCommand->setOperand(CmdActionMath::Operand1, oper1Var);
        }
        else
        {
            qreal oper1Val = mOper1Num->text().replace(",", ".").toDouble();
            mCommand->setOperand(CmdActionMath::Operand1, oper1Val);
        }
    }

    accept();
}

void CmdActionMathEditDialog::onCheckBoxStateChanged(int state)
{
    mOperationBox->setEnabled(state == Qt::Checked);
    mOperand2Box->setEnabled(state == Qt::Checked);
}

void CmdActionMathEditDialog::onOper1VarBtnStateChanged(bool toggled)
{
    mOper1Box->setEnabled(toggled);
    mOper1Num->setEnabled(!toggled);

    mOper1NumBtn->blockSignals(true);
    mOper1NumBtn->setChecked(false);
    mOper1NumBtn->blockSignals(false);
}

void CmdActionMathEditDialog::onOper1NumBtnStateChanged(bool toggled)
{
    mOper1Box->setEnabled(!toggled);
    mOper1Num->setEnabled(toggled);

    mOper1VarBtn->blockSignals(true);
    mOper1VarBtn->setChecked(false);
    mOper1VarBtn->blockSignals(false);
}

void CmdActionMathEditDialog::onOper2VarBtnStateChanged(bool toggled)
{
    mOper2Box->setEnabled(toggled);
    mOper2Num->setEnabled(!toggled);

    mOper2NumBtn->blockSignals(true);
    mOper2NumBtn->setChecked(false);
    mOper2NumBtn->blockSignals(false);
}

void CmdActionMathEditDialog::onOper2NumBtnStateChanged(bool toggled)
{
    mOper2Box->setEnabled(!toggled);
    mOper2Num->setEnabled(toggled);

    mOper2VarBtn->blockSignals(true);
    mOper2VarBtn->setChecked(false);
    mOper2VarBtn->blockSignals(false);
}


