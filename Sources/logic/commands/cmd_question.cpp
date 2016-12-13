#include "Headers/logic/commands/cmd_question.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/gui/cyclogram/valency_point.h"

#include <QTimer>

CmdQuestion::OperandData::OperandData()
{
    type = OperandNotSet;
}

//=================================

CmdQuestion::CmdQuestion(QObject* parent):
    Command(DRAKON::QUESTION, parent),
    mOperation(Equal),
    mOrientation(YesDown)
{
    updateText();
}

void CmdQuestion::run()
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

void CmdQuestion::swapBranches()
{
    if (mNextCommands.size() == 2)
    {
        foreach (Command* cmd, mNextCommands)
        {
            if (cmd->role() == ValencyPoint::Down)
            {
                cmd->setRole(ValencyPoint::Right);
            }
            else if (cmd->role() == ValencyPoint::Right)
            {
                cmd->setRole(ValencyPoint::Down);
            }
        }
    }
    else
    {
        qDebug("CmdQuestion warning: swapping branches does not performed (commands count=%i)", mNextCommands.size());
    }
}

void CmdQuestion::execute()
{
    // read current values from variable controller
    for (int i = 0; i < OperandsCount; ++i)
    {
        if (mOperands[i].type == Variable)
        {
            qreal v = mVarCtrl->variable(mOperands[i].variable);
            mOperands[i].value = v;
        }
    }

    bool result = false;

    // perform operation
    switch (mOperation)
    {
    case Greater:
        result = (mOperands[Left].value > mOperands[Right].value);
        break;
    case Less:
        result = (mOperands[Left].value < mOperands[Right].value);
        break;
    case Equal:
        // TODO здесь подстава подстав, так как из-за погрешностей округления циферки могут не сойтись и на строгое равенство проверять некорректно
        result = qAbs(mOperands[Left].value - mOperands[Right].value) < 0.01; // если числа различаются где-то в сотых, то считаем, что они равны
        break;
    case GreaterOrEqual:
        result = (mOperands[Left].value >= mOperands[Right].value);
        break;
    case LessOrEqual:
        result = (mOperands[Left].value <= mOperands[Right].value);
        break;
    case NotEqual:
        result = (mOperands[Left].value != mOperands[Right].value);
        break;

    default:
        break;
    }

    foreach (Command* cmd, mNextCommands)
    {
        bool cond1 = result && (mOrientation == YesDown) && (cmd->role() == ValencyPoint::Down);
        bool cond2 = result && (mOrientation == YesRight) && (cmd->role() == ValencyPoint::Right);
        bool cond3 = !result && (mOrientation == YesDown) && (cmd->role() == ValencyPoint::Right);
        bool cond4 = !result && (mOrientation == YesRight) && (cmd->role() == ValencyPoint::Down);

        if (cond1 || cond2 || cond3 || cond4)
        {
            emit finished(cmd);
            return;
        }
    }
}

void CmdQuestion::setOperation(Operation operation)
{
    mOperation = operation;
    updateText();
}

void CmdQuestion::setOrientation(Orientation orientation)
{
    if (mOrientation != orientation)
    {
        swapBranches();
    }

    mOrientation = orientation;
    updateText();
}

CmdQuestion::Orientation CmdQuestion::orientation() const
{
    return mOrientation;
}

void CmdQuestion::setOperand(OperandID operand, qreal value)
{
    if (operand < 0 || operand >= OperandsCount)
    {
        qDebug("Invalid operand input 1");
        return;
    }

    mOperands[operand].value = value;
    mOperands[operand].type = Number;
    mOperands[operand].variable = "";
    updateText();
}

void CmdQuestion::setOperand(OperandID operand, const QString& variable)
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

CmdQuestion::Operation CmdQuestion::operation() const
{
    return mOperation;
}

CmdQuestion::OperandType CmdQuestion::operandType(OperandID operand) const
{
    if (operand < 0 || operand >= OperandsCount)
    {
        qDebug("Invalid operand input 2");
        return OperandNotSet;
    }

    return mOperands[operand].type;
}

QString CmdQuestion::variableName(OperandID operand) const
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

qreal CmdQuestion::value(OperandID operand) const
{
    if (operand < 0 || operand >= OperandsCount)
    {
        qDebug("Invalid operand input 4");
        return -1;
    }

    return mOperands[operand].value;
}

void CmdQuestion::updateText()
{
    mText = "";

    bool isValid = true;

    switch (mOperands[Left].type)
    {
    case Variable:
        mText += mOperands[Left].variable;
        break;
    case Number:
        mText += QString::number(mOperands[Left].value);
        break;
    default:
        mText += "N/A";
        isValid = false;
        break;
    }

    switch (mOperation)
    {
    case Greater:
        mText += ">";
        break;
    case Less:
        mText += "<";
        break;
    case Equal:
        mText += "==";
        break;
    case GreaterOrEqual:
        mText += ">=";
        break;
    case LessOrEqual:
        mText += "<=";
        break;
    case NotEqual:
        mText += "!=";
        break;
    default:
        isValid = false;
        break;
    }

    switch (mOperands[Right].type)
    {
    case Variable:
        mText += mOperands[Right].variable;
        break;
    case Number:
        mText += QString::number(mOperands[Right].value);
        break;
    default:
        mText += "N/A";
        isValid = false;
        break;
    }

    if ((hasError() && isValid) || (!hasError() && !isValid))
    {
        setErrorStatus(!isValid);
    }

    emit textChanged(mText);
}

void CmdQuestion::onNameChanged(const QString& newName, const QString& oldName)
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

void CmdQuestion::onVariableRemoved(const QString& name)
{
    for (int i = 0; i < OperandsCount; ++i)
    {
        if (mOperands[i].type == Variable && mOperands[i].variable == name)
        {
            mOperands[i].type = OperandNotSet;
            mOperands[i].variable.clear();
        }
    }

    updateText();
}

CmdQuestion::QuestionType CmdQuestion::questionType() const
{
    return mQuestionType;
}

void CmdQuestion::setQuestionType(CmdQuestion::QuestionType type)
{
    mQuestionType = type;
}
