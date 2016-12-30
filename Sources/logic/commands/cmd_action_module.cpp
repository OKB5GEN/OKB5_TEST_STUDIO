#include "Headers/logic/commands/cmd_action_module.h"
//#include "Headers/logic/variable_controller.h"

#include <QTimer>

CmdActionModule::CmdActionModule(QObject* parent):
    CmdAction(DRAKON::ACTION_MODULE, parent)
//    mOperation(Assign)
{
    updateText();
}

void CmdActionModule::run()
{
    if (mExecutionDelay > 0)
    {
        QTimer::singleShot(mExecutionDelay, this, SLOT(execute()));
    }
    else
    {
        execute();
    }
}

void CmdActionModule::execute()
{
    // read current values from variable controller
/*    for (int i = 0; i < OperandsCount; ++i)
    {
        if (mOperands[i].type == Variable)
        {
            qreal v = mVarCtrl->variable(mOperands[i].variable);
            mOperands[i].value = v;
        }
    }

    // perform operation
    switch (mOperation)
    {
    case Add:
        mOperands[Result].value = mOperands[Operand1].value + mOperands[Operand2].value;
        break;
    case Subtract:
        mOperands[Result].value = mOperands[Operand1].value - mOperands[Operand2].value;
        break;
    case Multiply:
        mOperands[Result].value = mOperands[Operand1].value * mOperands[Operand2].value;
        break;
    case Divide:
        if (mOperands[Operand2].value != 0)
        {
            mOperands[Result].value = mOperands[Operand1].value / mOperands[Operand2].value;
        }
        else
        {
            mErrorText = tr("Division by zero in runtime");
            emit criticalError(this);
        }

        break;
    case Assign:
        mOperands[Result].value = mOperands[Operand1].value;
        break;
    default:
        break;
    }

    // set new variable value to variable controller
    mVarCtrl->setVariable(mOperands[Result].variable, mOperands[Result].value);

    finish();*/
}
/*
void CmdActionMath::setOperation(Operation operation)
{
    mOperation = operation;
    updateText();
}

void CmdActionMath::setOperand(OperandID operand, qreal value)
{
    if (operand < 0 || operand >= OperandsCount)
    {
        qDebug("Invalid operand input 1");
        return;
    }

    if (operand == Result)
    {
        qDebug("Operation result must be variable");
        return;
    }

    mOperands[operand].value = value;
    mOperands[operand].type = Number;
    mOperands[operand].variable = "";
    updateText();
}

void CmdActionMath::setOperand(OperandID operand, const QString& variable)
{
    if (operand < 0 || operand >= OperandsCount)
    {
        qDebug("Invalid operand input");
        return;
    }

    if (variable.isEmpty())
    {
        qDebug("No variable name provided");
        return;
    }

    mOperands[operand].value = mVarCtrl->variable(variable);
    mOperands[operand].type = Variable;
    mOperands[operand].variable = variable;
    updateText();
}

CmdActionMath::Operation CmdActionMath::operation() const
{
    return mOperation;
}

CmdActionMath::OperandType CmdActionMath::operandType(OperandID operand) const
{
    if (operand < 0 || operand >= OperandsCount)
    {
        qDebug("Invalid operand input 2");
        return OperandNotSet;
    }

    return mOperands[operand].type;
}

QString CmdActionMath::variableName(OperandID operand) const
{
    if (operand < 0 || operand >= OperandsCount)
    {
        qDebug("Invalid operand input 3");
        return "";
    }

    if (mOperands[operand].type == Number)
    {
        qDebug("Operand is not variable");
        return "";
    }

    return mOperands[operand].variable;
}

qreal CmdActionMath::value(OperandID operand) const
{
    if (operand < 0 || operand >= OperandsCount)
    {
        qDebug("Invalid operand input 4");
        return -1;
    }

    return mOperands[operand].value;
}
*/
void CmdActionModule::updateText()
{
    mText = "ModuleCMD";

    /*mText = "";

    if (mOperands[Result].variable.isEmpty())
    {
        mText = "N/A";
        if (!hasError())
        {
            setErrorStatus(true);
        }

        return;
    }

    bool isValid = true;

    mText += mOperands[Result].variable;
    mText += "=";

    switch (mOperands[Operand1].type)
    {
    case Variable:
        mText += mOperands[Operand1].variable;
        break;
    case Number:
        mText += QString::number(mOperands[Operand1].value);
        break;
    default:
        mText += "N/A";
        isValid = false;
        break;
    }

    if (mOperation != Assign)
    {
        switch (mOperation)
        {
        case Add:
            mText += "+";
            break;
        case Subtract:
            mText += "-";
            break;
        case Multiply:
            mText += "*";
            break;
        case Divide:
            mText += ":";
            break;
        default:
            break;
        }

        switch (mOperands[Operand2].type)
        {
        case Variable:
            mText += mOperands[Operand2].variable;
            break;
        case Number:
            mText += QString::number(mOperands[Operand2].value);
            break;
        default:
            mText += "N/A";
            isValid = false;
            break;
        }
    }

    if ((hasError() && isValid) || (!hasError() && !isValid))
    {
        setErrorStatus(!isValid);
    }*/

    emit textChanged(mText);
}

/*
void CmdActionMath::onNameChanged(const QString& newName, const QString& oldName)
{
    for (int i = 0; i < OperandsCount; ++i)
    {
        if (mOperands[i].type == Variable && mOperands[i].variable == oldName)
        {
            mOperands[i].variable = newName; // just change name
        }
    }

    updateText();
}

void CmdActionMath::onVariableRemoved(const QString& name)
{
    for (int i = 0; i < OperandsCount; ++i)
    {
        if (mOperands[i].type == Variable && mOperands[i].variable == name)
        {
            if (i == Operand2 && mOperation == Assign)
            {
                // do nothing
            }
            else
            {
                mOperands[i].type = OperandNotSet;
                mOperands[i].variable.clear();
            }
        }
    }

    updateText();
}*/
