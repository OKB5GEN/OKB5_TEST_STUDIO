#include "Headers/gui/cyclogram/dialogs/cmd_question_edit_dialog.h"
#include "Headers/logic/commands/cmd_question.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/gui/tools/console_text_widget.h"
#include "Headers/logger/Logger.h"

#include <QtWidgets>

CmdQuestionEditDialog::CmdQuestionEditDialog(QWidget * parent):
    RestorableDialog(parent),
    mCommand(Q_NULLPTR)
{
    setupUI();
    setWindowTitle(tr("Question"));

    adjustSize();
    setFixedSize(sizeHint());
}

CmdQuestionEditDialog::~CmdQuestionEditDialog()
{

}

void CmdQuestionEditDialog::setupUI()
{
    // TODO: убрать нафиг row/column span'ы у виджетов

    QGridLayout * layout = new QGridLayout(this);

    mValidator = new QDoubleValidator(this);

    // Operand 1 box >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    mOperand1Box = new QGroupBox(this);
    mOperand1Box->setTitle(tr("Operand 1"));
    QGridLayout* box1layout = new QGridLayout(mOperand1Box);

    mOper1VarBtn = new QRadioButton(this);
    mOper1NumBtn = new QRadioButton(this);
    mOper1Box = new QComboBox(this);
    mOper1Box->installEventFilter(this);
    mOper1Num = new QLineEdit(this);
    mOper1Num->setValidator(mValidator);

    box1layout->addWidget(mOper1VarBtn, 0, 0, 1, 1);
    box1layout->addWidget(mOper1NumBtn, 1, 0, 1, 1);
    box1layout->addWidget(mOper1Box, 0, 1, 1, 1);
    box1layout->addWidget(mOper1Num, 1, 1, 1, 1);

    mOperand1Box->setLayout(box1layout);
    layout->addWidget(mOperand1Box, 0, 0, 2, 2);

    // Operation box >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    QGroupBox* operationBox = new QGroupBox(this);
    operationBox->setTitle(tr("Operation"));
    QVBoxLayout* box3layout = new QVBoxLayout(operationBox);
    mOperationBox = new QComboBox(this);
    mOperationBox->installEventFilter(this);
    mOperationBox->addItem(">", QVariant(int(CmdQuestion::Greater)));
    mOperationBox->addItem("<", QVariant(int(CmdQuestion::Less)));
    mOperationBox->addItem("==", QVariant(int(CmdQuestion::Equal)));
    mOperationBox->addItem(">=", QVariant(int(CmdQuestion::GreaterOrEqual)));
    mOperationBox->addItem("<=", QVariant(int(CmdQuestion::LessOrEqual)));
    mOperationBox->addItem("!=", QVariant(int(CmdQuestion::NotEqual)));

    mYesDownBtn = new QRadioButton(this);
    mYesRightBtn = new QRadioButton(this);

    mYesDownBtn->setText(tr("Yes down"));
    mYesRightBtn->setText(tr("Yes right"));

    box3layout->addWidget(mOperationBox);
    box3layout->addWidget(mYesDownBtn);
    box3layout->addWidget(mYesRightBtn);

    operationBox->setLayout(box3layout);
    layout->addWidget(operationBox, 0, 2, 2, 2);

    // Operand 2 box >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    mOperand2Box = new QGroupBox(this);
    mOperand2Box->setTitle(tr("Operand 2"));
    QGridLayout* box2layout = new QGridLayout(mOperand2Box);

    mOper2VarBtn = new QRadioButton(this);
    mOper2NumBtn = new QRadioButton(this);
    mOper2Box = new QComboBox(this);
    mOper2Box->installEventFilter(this);
    mOper2Num = new QLineEdit(this);
    mOper2Num->setValidator(mValidator);

    box2layout->addWidget(mOper2VarBtn, 0, 0, 1, 1);
    box2layout->addWidget(mOper2NumBtn, 1, 0, 1, 1);
    box2layout->addWidget(mOper2Box, 0, 1, 1, 1);
    box2layout->addWidget(mOper2Num, 1, 1, 1, 1);

    mOperand2Box->setLayout(box2layout);

    layout->addWidget(mOperand2Box, 0, 4, 2, 2);

    // Console text widget >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    mConsoleTextWidget = new ConsoleTextWidget(this);
    layout->addWidget(mConsoleTextWidget, 3, 0, 1, 6);

    // Dialog button box >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 4, 4, 1, 2);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(layout);

    connect(mYesDownBtn, SIGNAL(toggled(bool)), this, SLOT(onYesDownBtnStateChanged(bool)));
    connect(mYesRightBtn, SIGNAL(toggled(bool)), this, SLOT(onYesRightBtnStateChanged(bool)));
    connect(mOper1VarBtn, SIGNAL(toggled(bool)), this, SLOT(onOper1VarBtnStateChanged(bool)));
    connect(mOper1NumBtn, SIGNAL(toggled(bool)), this, SLOT(onOper1NumBtnStateChanged(bool)));
    connect(mOper2VarBtn, SIGNAL(toggled(bool)), this, SLOT(onOper2VarBtnStateChanged(bool)));
    connect(mOper2NumBtn, SIGNAL(toggled(bool)), this, SLOT(onOper2NumBtnStateChanged(bool)));
}

void CmdQuestionEditDialog::setCommand(CmdQuestion* command)
{
    mCommand = command;

    if (mCommand)
    {
        // set default state
        mOper1VarBtn->setChecked(true);
        mOper2VarBtn->setChecked(true);
        mOper1Num->setText("0");
        mOper2Num->setText("0");
        mYesDownBtn->setChecked(mCommand->orientation() == CmdQuestion::YesDown);
        mYesRightBtn->setChecked(mCommand->orientation() == CmdQuestion::YesRight);

        mOper1Box->clear();
        mOper2Box->clear();

        VariableController* vc = mCommand->variableController();
        mOper1Box->addItems(vc->variablesData().keys());
        mOper2Box->addItems(vc->variablesData().keys());

        int index = mOperationBox->findData(QVariant(int(mCommand->operation())));
        if (index != -1)
        {
            mOperationBox->setCurrentIndex(index);
        }

        updateComponent(CmdQuestion::Left, mOper1Box, mOper1Num, mOper1VarBtn, mOper1NumBtn);
        updateComponent(CmdQuestion::Right, mOper2Box, mOper2Num, mOper2VarBtn, mOper2NumBtn);

        mConsoleTextWidget->setCommand(mCommand);
    }

    readSettings();
}

void CmdQuestionEditDialog::updateComponent(int operand, QComboBox* box, QLineEdit* lineEdit, QRadioButton* boxBtn, QRadioButton* lineEditBtn)
{
    VariableController* vc = mCommand->variableController();
    CmdQuestion::OperandID op = CmdQuestion::OperandID(operand);

    if (mCommand->operandType(op) == CmdQuestion::Variable)
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
    else if (mCommand->operandType(op) == CmdQuestion::Number)
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
}

void CmdQuestionEditDialog::onAccept()
{
    if (!mCommand)
    {
        LOG_ERROR(QString("No question command set!"));
        accept();
        return;
    }

    CmdQuestion::Operation operation = CmdQuestion::Operation(mOperationBox->currentData().toInt());
    CmdQuestion::Orientation orientation;

    CmdQuestion::OperandData left;
    CmdQuestion::OperandData right;

    if (mOper1VarBtn->isChecked())
    {
        left.type = CmdQuestion::Variable;
        left.variable = mOper1Box->currentText();
        left.value = 0;
    }
    else
    {
        left.type = CmdQuestion::Number;
        left.variable = "";
        left.value = mOper1Num->text().replace(",", ".").toDouble();
    }

    if (mOper2VarBtn->isChecked())
    {
        right.type = CmdQuestion::Variable;
        right.variable = mOper2Box->currentText();
        right.value = 0;
    }
    else
    {
        right.type = CmdQuestion::Number;
        right.variable = "";
        right.value = mOper2Num->text().replace(",", ".").toDouble();
    }

    if (mYesDownBtn->isChecked())
    {
        orientation = CmdQuestion::YesDown;
    }
    else if (mYesRightBtn->isChecked())
    {
        orientation = CmdQuestion::YesRight;
    }

    mCommand->setData(operation, orientation, left, right);
    mConsoleTextWidget->saveCommand();

    accept();
}

void CmdQuestionEditDialog::onOper1VarBtnStateChanged(bool toggled)
{
    mOper1Box->setEnabled(toggled);
    mOper1Num->setEnabled(!toggled);

    mOper1NumBtn->blockSignals(true);
    mOper1NumBtn->setChecked(false);
    mOper1NumBtn->blockSignals(false);
}

void CmdQuestionEditDialog::onOper1NumBtnStateChanged(bool toggled)
{
    mOper1Box->setEnabled(!toggled);
    mOper1Num->setEnabled(toggled);

    mOper1VarBtn->blockSignals(true);
    mOper1VarBtn->setChecked(false);
    mOper1VarBtn->blockSignals(false);
}

void CmdQuestionEditDialog::onOper2VarBtnStateChanged(bool toggled)
{
    mOper2Box->setEnabled(toggled);
    mOper2Num->setEnabled(!toggled);

    mOper2NumBtn->blockSignals(true);
    mOper2NumBtn->setChecked(false);
    mOper2NumBtn->blockSignals(false);
}

void CmdQuestionEditDialog::onOper2NumBtnStateChanged(bool toggled)
{
    mOper2Box->setEnabled(!toggled);
    mOper2Num->setEnabled(toggled);

    mOper2VarBtn->blockSignals(true);
    mOper2VarBtn->setChecked(false);
    mOper2VarBtn->blockSignals(false);
}

void CmdQuestionEditDialog::onYesDownBtnStateChanged(bool toggled)
{
    if (toggled)
    {
        mYesRightBtn->blockSignals(true);
        mYesRightBtn->setChecked(false);
        mYesRightBtn->blockSignals(false);
    }
}

void CmdQuestionEditDialog::onYesRightBtnStateChanged(bool toggled)
{
    if (toggled)
    {
        mYesDownBtn->blockSignals(true);
        mYesDownBtn->setChecked(false);
        mYesDownBtn->blockSignals(false);
    }
}

bool CmdQuestionEditDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Wheel && qobject_cast<QComboBox*>(obj))
    {
        return true; // do not process wheel events if combo box is not "expanded/opened"
    }
    else
    {
        return QObject::eventFilter(obj, event); // standard event processing
    }
}
