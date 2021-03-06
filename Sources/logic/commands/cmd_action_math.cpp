#include "Headers/logic/commands/cmd_action_math.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMetaEnum>

CmdActionMath::OperandData::OperandData()
{
    type = OperandNotSet;
}

///

CmdActionMath::CmdActionMath(QObject* parent):
    Command(DRAKON::ACTION_MATH, 1, parent),
    mOperation(Assign)
{
    updateText();
}

void CmdActionMath::run()
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

void CmdActionMath::execute()
{
    // read current values from variable controller
    for (int i = 0; i < OperandsCount; ++i)
    {
        if (mOperands[i].type == Variable)
        {
            qreal v = mVarCtrl->currentValue(mOperands[i].variable);
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
    mVarCtrl->setCurrentValue(mOperands[Result].variable, mOperands[Result].value);

    mVarCtrl->makeDataSnapshot(); //TODO

    emit finished(nextCommand()); // command succesfully executed
}

void CmdActionMath::setOperation(Operation operation)
{
    mOperation = operation;
    updateText();
}

void CmdActionMath::setOperand(OperandID operand, qreal value)
{
    if (operand < 0 || operand >= OperandsCount)
    {
        LOG_WARNING(QString("Invalid operand input 1"));
        return;
    }

    if (operand == Result)
    {
        LOG_WARNING(QString("Operation result must be variable"));
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
        LOG_WARNING(QString("Invalid operand input"));
        return;
    }

    if (variable.isEmpty())
    {
        LOG_WARNING(QString("No variable name provided"));
        return;
    }

    mOperands[operand].value = mVarCtrl->currentValue(variable);
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
        LOG_WARNING(QString("Invalid operand input 2"));
        return OperandNotSet;
    }

    return mOperands[operand].type;
}

QString CmdActionMath::variableName(OperandID operand) const
{
    if (operand < 0 || operand >= OperandsCount)
    {
        LOG_WARNING(QString("Invalid operand input 3"));
        return "";
    }

    if (mOperands[operand].type == Number)
    {
        LOG_WARNING(QString("Operand is not variable"));
        return "";
    }

    return mOperands[operand].variable;
}

qreal CmdActionMath::value(OperandID operand) const
{
    if (operand < 0 || operand >= OperandsCount)
    {
        LOG_WARNING(QString("Invalid operand input 4"));
        return -1;
    }

    return mOperands[operand].value;
}

void CmdActionMath::updateText()
{
    QString textBefore = mText;

    mText = "";

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
    mText += " = ";

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
            mText += " + ";
            break;
        case Subtract:
            mText += " - ";
            break;
        case Multiply:
            mText += " * ";
            break;
        case Divide:
            mText += " : ";
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
    }

    if (textBefore != mText)
    {
        emit dataChanged(mText);
    }
}

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
}

void CmdActionMath::writeCustomAttributes(QXmlStreamWriter* writer)
{
    QMetaEnum operation = QMetaEnum::fromType<CmdActionMath::Operation>();
    QMetaEnum operandType = QMetaEnum::fromType<CmdActionMath::OperandType>();
    QMetaEnum operandId = QMetaEnum::fromType<CmdActionMath::OperandID>();

    writer->writeAttribute("operation", operation.valueToKey(mOperation));

    for (int i = 0; i < OperandsCount; ++i)
    {
        writer->writeStartElement("operand");
        writer->writeAttribute("id", operandId.valueToKey(OperandID(i)));
        writer->writeAttribute("type", operandType.valueToKey(mOperands[i].type));
        writer->writeAttribute("value", QString::number(mOperands[i].value));
        writer->writeAttribute("variable", mOperands[i].variable);
        writer->writeEndElement();
    }
}

void CmdActionMath::readCustomAttributes(QXmlStreamReader* reader, const Version& fileVersion)
{
    QMetaEnum operation = QMetaEnum::fromType<CmdActionMath::Operation>();
    QMetaEnum operandType = QMetaEnum::fromType<CmdActionMath::OperandType>();
    QMetaEnum operandId = QMetaEnum::fromType<CmdActionMath::OperandID>();

    QXmlStreamAttributes attributes = reader->attributes();
    if (attributes.hasAttribute("operation"))
    {
        QString str = attributes.value("operation").toString();
        mOperation = Operation(operation.keyToValue(qPrintable(str)));
    }

    while (!(reader->tokenType() == QXmlStreamReader::EndElement && reader->name() == "command"))
    {
        if (reader->tokenType() == QXmlStreamReader::StartElement && reader->name() == "operand")
        {
            attributes = reader->attributes();
            OperandID id;
            qreal value;
            OperandType type;
            QString variable;

            if (attributes.hasAttribute("id"))
            {
                QString str = attributes.value("id").toString();
                id = OperandID(operandId.keyToValue(qPrintable(str)));
            }

            if (attributes.hasAttribute("type"))
            {
                QString str = attributes.value("type").toString();
                type = OperandType(operandType.keyToValue(qPrintable(str)));
            }

            if (attributes.hasAttribute("value"))
            {
                value = attributes.value("value").toDouble();
            }

            if (attributes.hasAttribute("variable"))
            {
                variable = attributes.value("variable").toString();
            }

            mOperands[id].type = type;
            mOperands[id].value = value;
            mOperands[id].variable = variable;
        }

        reader->readNext();
    }

    updateText();
}

bool CmdActionMath::loadFromImpl(Command* other)
{
    CmdActionMath* otherMath = qobject_cast<CmdActionMath*>(other);
    if (!otherMath)
    {
        LOG_ERROR(QString("Command type mismatch (not math)"));
        return false;
    }

    mOperation = otherMath->operation();
    mOperands[Result] = otherMath->mOperands[Result];
    mOperands[Operand1] = otherMath->mOperands[Operand1];
    mOperands[Operand2] = otherMath->mOperands[Operand2];

    return true;
}

bool CmdActionMath::isVariableUsed(const QString& name) const
{
    for (int i = 0; i < OperandsCount; ++i)
    {
        if (mOperands[i].type == Variable && mOperands[i].variable == name)
        {
            return true;
        }
    }

    return false;
}
