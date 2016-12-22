#include "Headers/logic/commands/cmd_question.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/gui/cyclogram/valency_point.h"

#include <QTimer>

namespace
{
    static const qreal PRECISION = 0.001;
}

CmdQuestion::OperandData::OperandData()
{
    type = OperandNotSet;
}

//=================================

CmdQuestion::CmdQuestion(QObject* parent):
    Command(DRAKON::QUESTION, 3, parent),
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

void CmdQuestion::execute()
{
    // TODO
    /* Логика выполняется в вещественных числах, поэтому все операции, где есть проверка на равенство могут в некоторых ситуациях работать некорректно
     * Надо либо разделять "дробные/целые" и дробные сранивать только на больше-меньше
    */

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
        result = (qAbs(mOperands[Left].value - mOperands[Right].value) < PRECISION);
        break;
    case GreaterOrEqual:
        result = ((mOperands[Left].value > mOperands[Right].value) || qAbs(mOperands[Left].value - mOperands[Right].value) < PRECISION);
        break;
    case LessOrEqual:
        result = ((mOperands[Left].value < mOperands[Right].value) || qAbs(mOperands[Left].value - mOperands[Right].value) < PRECISION);
        break;
    case NotEqual:
        result = (qAbs(mOperands[Left].value - mOperands[Right].value) > PRECISION);
        break;

    default:
        break;
    }

    Command* cmd = Q_NULLPTR;
    Command* right = nextCommand(ValencyPoint::Right);
    Command* down = nextCommand(ValencyPoint::Down);

    if (result)
    {
        cmd = nextCommand((mOrientation == YesDown) ? ValencyPoint::Down : ValencyPoint::Right);
    }
    else
    {
        cmd = nextCommand((mOrientation == YesDown) ? ValencyPoint::Right : ValencyPoint::Down);
    }

    emit finished(cmd);
}

void CmdQuestion::setOperation(Operation operation)
{
    mOperation = operation;
    updateText();
}

void CmdQuestion::setOrientation(Orientation orientation)
{
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
