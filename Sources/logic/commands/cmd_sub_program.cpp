#include "Headers/logic/commands/cmd_sub_program.h"

//#include "Headers/logic/variable_controller.h"

#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMetaEnum>

CmdSubProgram::CmdSubProgram(QObject* parent):
    CmdAction(DRAKON::SUBPROGRAM, parent)
{
    updateText();
}

void CmdSubProgram::run()
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

void CmdSubProgram::execute()
{
//    // read current values from variable controller
//    for (int i = 0; i < OperandsCount; ++i)
//    {
//        if (mOperands[i].type == Variable)
//        {
//            qreal v = mVarCtrl->variable(mOperands[i].variable);
//            mOperands[i].value = v;
//        }
//    }

//    // perform operation
//    switch (mOperation)
//    {
//    case Add:
//        mOperands[Result].value = mOperands[Operand1].value + mOperands[Operand2].value;
//        break;
//    case Subtract:
//        mOperands[Result].value = mOperands[Operand1].value - mOperands[Operand2].value;
//        break;
//    case Multiply:
//        mOperands[Result].value = mOperands[Operand1].value * mOperands[Operand2].value;
//        break;
//    case Divide:
//        if (mOperands[Operand2].value != 0)
//        {
//            mOperands[Result].value = mOperands[Operand1].value / mOperands[Operand2].value;
//        }
//        else
//        {
//            mErrorText = tr("Division by zero in runtime");
//            emit criticalError(this);
//        }

//        break;
//    case Assign:
//        mOperands[Result].value = mOperands[Operand1].value;
//        break;
//    default:
//        break;
//    }

//    // set new variable value to variable controller
//    mVarCtrl->setVariable(mOperands[Result].variable, mOperands[Result].value);

    finish();
}

void CmdSubProgram::updateText()
{
    mText = "";

//    if (mOperands[Result].variable.isEmpty())
//    {
//        mText = "N/A";
//        if (!hasError())
//        {
//            setErrorStatus(true);
//        }

//        return;
//    }

//    bool isValid = true;

//    mText += mOperands[Result].variable;
//    mText += "=";

//    switch (mOperands[Operand1].type)
//    {
//    case Variable:
//        mText += mOperands[Operand1].variable;
//        break;
//    case Number:
//        mText += QString::number(mOperands[Operand1].value);
//        break;
//    default:
//        mText += "N/A";
//        isValid = false;
//        break;
//    }

//    if (mOperation != Assign)
//    {
//        switch (mOperation)
//        {
//        case Add:
//            mText += "+";
//            break;
//        case Subtract:
//            mText += "-";
//            break;
//        case Multiply:
//            mText += "*";
//            break;
//        case Divide:
//            mText += ":";
//            break;
//        default:
//            break;
//        }

//        switch (mOperands[Operand2].type)
//        {
//        case Variable:
//            mText += mOperands[Operand2].variable;
//            break;
//        case Number:
//            mText += QString::number(mOperands[Operand2].value);
//            break;
//        default:
//            mText += "N/A";
//            isValid = false;
//            break;
//        }
//    }

//    if ((hasError() && isValid) || (!hasError() && !isValid))
//    {
//        setErrorStatus(!isValid);
//    }

    emit textChanged(mText);
}

void CmdSubProgram::onNameChanged(const QString& newName, const QString& oldName)
{
//    for (int i = 0; i < OperandsCount; ++i)
//    {
//        if (mOperands[i].type == Variable && mOperands[i].variable == oldName)
//        {
//            mOperands[i].variable = newName; // just change name
//        }
//    }

//    updateText();
}

void CmdSubProgram::onVariableRemoved(const QString& name)
{
//    for (int i = 0; i < OperandsCount; ++i)
//    {
//        if (mOperands[i].type == Variable && mOperands[i].variable == name)
//        {
//            if (i == Operand2 && mOperation == Assign)
//            {
//                // do nothing
//            }
//            else
//            {
//                mOperands[i].type = OperandNotSet;
//                mOperands[i].variable.clear();
//            }
//        }
//    }

//    updateText();
}

void CmdSubProgram::writeCustomAttributes(QXmlStreamWriter* writer)
{
//    QMetaEnum operation = QMetaEnum::fromType<CmdSubProgram::Operation>();
//    QMetaEnum operandType = QMetaEnum::fromType<CmdSubProgram::OperandType>();
//    QMetaEnum operandId = QMetaEnum::fromType<CmdSubProgram::OperandID>();

//    writer->writeAttribute("operation", operation.valueToKey(mOperation));

//    for (int i = 0; i < OperandsCount; ++i)
//    {
//        writer->writeStartElement("operand");
//        writer->writeAttribute("id", operandId.valueToKey(OperandID(i)));
//        writer->writeAttribute("type", operandType.valueToKey(mOperands[i].type));
//        writer->writeAttribute("value", QString::number(mOperands[i].value));
//        writer->writeAttribute("variable", mOperands[i].variable);
//        writer->writeEndElement();
//    }
}

void CmdSubProgram::readCustomAttributes(QXmlStreamReader* reader)
{
//    QMetaEnum operation = QMetaEnum::fromType<CmdSubProgram::Operation>();
//    QMetaEnum operandType = QMetaEnum::fromType<CmdSubProgram::OperandType>();
//    QMetaEnum operandId = QMetaEnum::fromType<CmdSubProgram::OperandID>();

//    QXmlStreamAttributes attributes = reader->attributes();
//    if (attributes.hasAttribute("operation"))
//    {
//        QString str = attributes.value("operation").toString();
//        mOperation = Operation(operation.keyToValue(qPrintable(str)));
//    }

//    while (!(reader->tokenType() == QXmlStreamReader::EndElement && reader->name() == "command"))
//    {
//        if (reader->tokenType() == QXmlStreamReader::StartElement && reader->name() == "operand")
//        {
//            attributes = reader->attributes();
//            OperandID id;
//            qreal value;
//            OperandType type;
//            QString variable;

//            if (attributes.hasAttribute("id"))
//            {
//                QString str = attributes.value("id").toString();
//                id = OperandID(operandId.keyToValue(qPrintable(str)));
//            }

//            if (attributes.hasAttribute("type"))
//            {
//                QString str = attributes.value("type").toString();
//                type = OperandType(operandType.keyToValue(qPrintable(str)));
//            }

//            if (attributes.hasAttribute("value"))
//            {
//                value = attributes.value("value").toDouble();
//            }

//            if (attributes.hasAttribute("variable"))
//            {
//                variable = attributes.value("variable").toString();
//            }

//            mOperands[id].type = type;
//            mOperands[id].value = value;
//            mOperands[id].variable = variable;
//        }

//        reader->readNext();
//    }

//    updateText();
}
