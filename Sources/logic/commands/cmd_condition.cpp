#include "Headers/logic/commands/cmd_condition.h"
//#include "Headers/logic/variable_controller.h"
//#include "Headers/gui/cyclogram/valency_point.h"
//#include "Headers/logger/Logger.h"
//#include <QTimer>
//#include <QXmlStreamWriter>
//#include <QXmlStreamReader>
//#include <QMetaEnum>

//namespace
//{
//    static const qreal PRECISION = 0.001;
//}

//CmdCondition::OperandData::OperandData()
//{
//    type = OperandNotSet;
//}

//=================================

CmdCondition::CmdCondition(QObject* parent):
    CmdQuestion(DRAKON::CONDITION, 3, parent)//,
//    mOperation(Equal),
//    mOrientation(YesDown)
{
//    updateText();
}

//void CmdQuestion::run()
//{
//    if (mExecutionDelay > 0)
//    {
//        QTimer::singleShot(mExecutionDelay, this, SLOT(execute()));
//    }
//    else
//    {
//        execute();
//    }
//}

//void CmdQuestion::execute()
//{
//    // TODO
//    /* Логика выполняется в вещественных числах, поэтому все операции, где есть проверка на равенство могут в некоторых ситуациях работать некорректно
//     * Надо либо разделять "дробные/целые" и дробные сранивать только на больше-меньше
//    */

//    // read current values from variable controller
//    for (int i = 0; i < OperandsCount; ++i)
//    {
//        if (mOperands[i].type == Variable)
//        {
//            qreal v = mVarCtrl->currentValue(mOperands[i].variable);
//            mOperands[i].value = v;
//        }
//    }

//    bool result = false;

//    // perform operation
//    switch (mOperation)
//    {
//    case Greater:
//        result = (mOperands[Left].value > mOperands[Right].value);
//        break;
//    case Less:
//        result = (mOperands[Left].value < mOperands[Right].value);
//        break;
//    case Equal:
//        result = (qAbs(mOperands[Left].value - mOperands[Right].value) < PRECISION);
//        break;
//    case GreaterOrEqual:
//        result = ((mOperands[Left].value > mOperands[Right].value) || qAbs(mOperands[Left].value - mOperands[Right].value) < PRECISION);
//        break;
//    case LessOrEqual:
//        result = ((mOperands[Left].value < mOperands[Right].value) || qAbs(mOperands[Left].value - mOperands[Right].value) < PRECISION);
//        break;
//    case NotEqual:
//        result = (qAbs(mOperands[Left].value - mOperands[Right].value) > PRECISION);
//        break;

//    default:
//        break;
//    }

//    Command* cmd = Q_NULLPTR;
//    Command* right = nextCommand(ValencyPoint::Right);
//    Command* down = nextCommand(ValencyPoint::Down);

//    LOG_INFO(QString("Result is: %1").arg(result ? "YES" : "NO"));

//    if (result)
//    {
//        cmd = nextCommand((mOrientation == YesDown) ? ValencyPoint::Down : ValencyPoint::Right);
//    }
//    else
//    {
//        cmd = nextCommand((mOrientation == YesDown) ? ValencyPoint::Right : ValencyPoint::Down);
//    }

//    emit finished(cmd);
//}

//void CmdQuestion::setOperation(Operation operation)
//{
//    mOperation = operation;
//    updateText();
//}

//void CmdQuestion::setOrientation(Orientation orientation)
//{
//    mOrientation = orientation;
//    updateText();
//}

//CmdQuestion::Orientation CmdQuestion::orientation() const
//{
//    return mOrientation;
//}

//void CmdQuestion::setOperand(OperandID operand, qreal value)
//{
//    if (operand < 0 || operand >= OperandsCount)
//    {
//        LOG_WARNING(QString("Invalid operand input 1"));
//        return;
//    }

//    mOperands[operand].value = value;
//    mOperands[operand].type = Number;
//    mOperands[operand].variable = "";
//    updateText();
//}

//void CmdQuestion::setOperand(OperandID operand, const QString& variable)
//{
//    if (operand < 0 || operand >= OperandsCount)
//    {
//        LOG_WARNING(QString("Invalid operand input"));
//        return;
//    }

//    if (variable.isEmpty())
//    {
//        LOG_WARNING(QString("No variable name provided"));
//        return;
//    }

//    mOperands[operand].value = mVarCtrl->currentValue(variable);
//    mOperands[operand].type = Variable;
//    mOperands[operand].variable = variable;
//    updateText();
//}

//CmdQuestion::Operation CmdQuestion::operation() const
//{
//    return mOperation;
//}

//CmdQuestion::OperandType CmdQuestion::operandType(OperandID operand) const
//{
//    if (operand < 0 || operand >= OperandsCount)
//    {
//        LOG_WARNING(QString("Invalid operand input 2"));
//        return OperandNotSet;
//    }

//    return mOperands[operand].type;
//}

//QString CmdQuestion::variableName(OperandID operand) const
//{
//    if (operand < 0 || operand >= OperandsCount)
//    {
//        LOG_WARNING(QString("Invalid operand input 3"));
//        return "";
//    }

//    if (mOperands[operand].type == Number)
//    {
//        LOG_WARNING(QString("Operand is not variable"));
//        return "";
//    }

//    return mOperands[operand].variable;
//}

//qreal CmdQuestion::value(OperandID operand) const
//{
//    if (operand < 0 || operand >= OperandsCount)
//    {
//        LOG_WARNING(QString("Invalid operand input 4"));
//        return -1;
//    }

//    return mOperands[operand].value;
//}

//void CmdQuestion::updateText()
//{
//    mText = "";

//    bool isValid = true;

//    switch (mOperands[Left].type)
//    {
//    case Variable:
//        {
//            if (mOperands[Left].variable.isEmpty())
//            {
//                mText += "N/A";
//                isValid = false;
//            }
//            else
//            {
//                mText += mOperands[Left].variable;
//            }
//        }
//        break;
//    case Number:
//        mText += QString::number(mOperands[Left].value);
//        break;
//    default:
//        mText += "N/A";
//        isValid = false;
//        break;
//    }

//    switch (mOperation)
//    {
//    case Greater:
//        mText += ">";
//        break;
//    case Less:
//        mText += "<";
//        break;
//    case Equal:
//        mText += "==";
//        break;
//    case GreaterOrEqual:
//        mText += ">=";
//        break;
//    case LessOrEqual:
//        mText += "<=";
//        break;
//    case NotEqual:
//        mText += "!=";
//        break;
//    default:
//        isValid = false;
//        break;
//    }

//    switch (mOperands[Right].type)
//    {
//    case Variable:
//        if (mOperands[Right].variable.isEmpty())
//        {
//            mText += "N/A";
//            isValid = false;
//        }
//        else
//        {
//            mText += mOperands[Right].variable;
//        }
//        break;
//    case Number:
//        mText += QString::number(mOperands[Right].value);
//        break;
//    default:
//        mText += "N/A";
//        isValid = false;
//        break;
//    }

//    if ((hasError() && isValid) || (!hasError() && !isValid))
//    {
//        setErrorStatus(!isValid);
//    }

//    emit dataChanged(mText);
//}

//void CmdQuestion::onNameChanged(const QString& newName, const QString& oldName)
//{
//    for (int i = 0; i < OperandsCount; ++i)
//    {
//        if (mOperands[i].type == Variable && mOperands[i].variable == oldName)
//        {
//            mOperands[i].variable = newName; // just change name
//        }
//    }

//    updateText();
//}

//void CmdQuestion::onVariableRemoved(const QString& name)
//{
//    for (int i = 0; i < OperandsCount; ++i)
//    {
//        if (mOperands[i].type == Variable && mOperands[i].variable == name)
//        {
//            mOperands[i].type = OperandNotSet;
//            mOperands[i].variable.clear();
//        }
//    }

//    updateText();
//}

//void CmdQuestion::setData(Operation operation, Orientation orientation, const OperandData& left, const OperandData& right)
//{
//    Operation operationBefore = mOperation;
//    Orientation orientationBefore = mOrientation;
//    OperandData leftBefore = mOperands[Left];
//    OperandData rightBefore = mOperands[Right];

//    mOperation = operation;
//    mOrientation = orientation;
//    mOperands[Left] = left;
//    mOperands[Right] = right;

//    bool isDataChanged = ((operationBefore != mOperation)
//                          || (orientationBefore != mOrientation)
//                          || (leftBefore.variable != mOperands[Left].variable)
//                          || (leftBefore.value != mOperands[Left].value)
//                          || (leftBefore.type != mOperands[Left].type)
//                          || (rightBefore.variable != mOperands[Right].variable)
//                          || (rightBefore.value != mOperands[Right].value)
//                          || (rightBefore.type != mOperands[Right].type)
//                          );

//    if (isDataChanged)
//    {
//        updateText();
//    }
//}

//CmdQuestion::QuestionType CmdQuestion::questionType() const
//{
//    return mQuestionType;
//}

//void CmdQuestion::setQuestionType(CmdQuestion::QuestionType type)
//{
//    mQuestionType = type;
//}

//void CmdQuestion::writeCustomAttributes(QXmlStreamWriter* writer)
//{
//    QMetaEnum cmdType = QMetaEnum::fromType<CmdQuestion::QuestionType>();
//    QMetaEnum orientation = QMetaEnum::fromType<CmdQuestion::Orientation>();
//    QMetaEnum operation = QMetaEnum::fromType<CmdQuestion::Operation>();
//    QMetaEnum operandId = QMetaEnum::fromType<CmdQuestion::OperandID>();
//    QMetaEnum operandType = QMetaEnum::fromType<CmdQuestion::OperandType>();

//    writer->writeAttribute("operation", operation.valueToKey(mOperation));
//    writer->writeAttribute("orientation", orientation.valueToKey(mOrientation));
//    writer->writeAttribute("cmd_type", cmdType.valueToKey(mQuestionType));

//    for (int i = 0; i < OperandsCount; ++i)
//    {
//        writer->writeStartElement("operand");
//        writer->writeAttribute("id", operandId.valueToKey(OperandID(i)));
//        writer->writeAttribute("type", operandType.valueToKey(mOperands[i].type));
//        writer->writeAttribute("value", QString::number(mOperands[i].value));
//        writer->writeAttribute("variable", mOperands[i].variable);
//        writer->writeEndElement();
//    }
//}

//void CmdQuestion::readCustomAttributes(QXmlStreamReader* reader, const AppVersion& fileVersion)
//{
//    QMetaEnum cmdType = QMetaEnum::fromType<CmdQuestion::QuestionType>();
//    QMetaEnum orientation = QMetaEnum::fromType<CmdQuestion::Orientation>();
//    QMetaEnum operation = QMetaEnum::fromType<CmdQuestion::Operation>();
//    QMetaEnum operandId = QMetaEnum::fromType<CmdQuestion::OperandID>();
//    QMetaEnum operandType = QMetaEnum::fromType<CmdQuestion::OperandType>();

//    QXmlStreamAttributes attributes = reader->attributes();
//    if (attributes.hasAttribute("operation"))
//    {
//        QString str = attributes.value("operation").toString();
//        mOperation = Operation(operation.keyToValue(qPrintable(str)));
//    }

//    if (attributes.hasAttribute("orientation"))
//    {
//        QString str = attributes.value("orientation").toString();
//        mOrientation = Orientation(orientation.keyToValue(qPrintable(str)));
//    }

//    if (attributes.hasAttribute("cmd_type"))
//    {
//        QString str = attributes.value("cmd_type").toString();
//        mQuestionType = QuestionType(cmdType.keyToValue(qPrintable(str)));
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
//}

void CmdCondition::insertCommand(Command* newCmd, ValencyPoint::Role role)
{
    insertInIf(newCmd, role);
}

void CmdCondition::insertInIf(Command* newCmd, ValencyPoint::Role role)
{
    if (newCmd->type() == DRAKON::CYCLE)
    {
        insertCycleToIf(newCmd, role);
        return;
    }

    if (newCmd->type() == DRAKON::CONDITION)
    {
        insertIfToIf(newCmd, role);
        return;
    }

    if (newCmd->type() == DRAKON::SELECT_STATE)
    {
        insertSwitchStateToIf(newCmd, role);
        return;
    }

    // simple command insertion
    Command* underArrow = nextCommand(ValencyPoint::UnderArrow);
    Command* right = nextCommand(ValencyPoint::Right);
    Command* down = nextCommand(ValencyPoint::Down);

    if (role == ValencyPoint::UnderArrow)
    {
        // recursively replace references to current "under arrow" command with new "under arrow" command
        if (down == underArrow)
        {
            mNextCommands[ValencyPoint::Down] = newCmd;
        }
        else
        {
            replaceReferences(underArrow, newCmd, down);
        }

        if (right == underArrow)
        {
            mNextCommands[ValencyPoint::Right] = newCmd;
        }
        else
        {
            replaceReferences(underArrow, newCmd, right);
        }

        newCmd->replaceCommand(underArrow, ValencyPoint::Down);
    }
    else
    {
        Command* existing = mNextCommands[role];
        Command* cmd = (existing == underArrow) ? underArrow : existing;

        newCmd->replaceCommand(cmd, ValencyPoint::Down);
    }

    mNextCommands[role] = newCmd;
}

void CmdCondition::insertCycleToIf(Command* newCmd, ValencyPoint::Role role)
{
    int TODO; // need check + хз как разруливать дальнейшие добавления даже простых команд во вложенный under arrow? (может replace references этим занимается)

    Command* underArrow = nextCommand(ValencyPoint::UnderArrow);
    Command* right = nextCommand(ValencyPoint::Right);
    Command* down = nextCommand(ValencyPoint::Down);

    if (role == ValencyPoint::UnderArrow)
    {
        // recursively replace references to current "under arrow" command with new "under arrow" command
        if (down == underArrow)
        {
            mNextCommands[ValencyPoint::Down] = newCmd;
        }
        else
        {
            replaceReferences(underArrow, newCmd, down);
        }

        if (right == underArrow)
        {
            mNextCommands[ValencyPoint::Right] = newCmd;
        }
        else
        {
            replaceReferences(underArrow, newCmd, right);
        }

        // old "under arrow" command now become "down" command for new command, "right" and "under arrow" link command to itself
        newCmd->replaceCommand(underArrow, ValencyPoint::Down);
    }
    else
    {
        Command* existing = mNextCommands[role];
        Command* cmd = (existing == underArrow) ? underArrow : existing;

        newCmd->replaceCommand(cmd, ValencyPoint::Down);
    }

    newCmd->replaceCommand(newCmd, ValencyPoint::Right);
    newCmd->replaceCommand(newCmd, ValencyPoint::UnderArrow);
    mNextCommands[role] = newCmd;
}

void CmdCondition::insertIfToIf(Command* newCmd, ValencyPoint::Role role)
{
    Command* underArrow = nextCommand(ValencyPoint::UnderArrow);
    Command* right = nextCommand(ValencyPoint::Right);
    Command* down = nextCommand(ValencyPoint::Down);

    Command* cmd = Q_NULLPTR;

    if (role == ValencyPoint::UnderArrow)
    {
        cmd = underArrow;

        // recursively replace references to current "under arrow" command with new "under arrow" command
        if (down == underArrow)
        {
            mNextCommands[ValencyPoint::Down] = newCmd;
        }
        else
        {
            replaceReferences(underArrow, newCmd, down);
        }

        if (right == underArrow)
        {
            mNextCommands[ValencyPoint::Right] = newCmd;
        }
        else
        {
            replaceReferences(underArrow, newCmd, right);
        }
    }
    else
    {
        Command* existing = mNextCommands[role];
        cmd = (existing == underArrow) ? underArrow : existing;
    }

    // old "under arrow" command now become "under arrow, etc" command for new command
    newCmd->replaceCommand(cmd, ValencyPoint::Down);
    newCmd->replaceCommand(cmd, ValencyPoint::Right);
    newCmd->replaceCommand(cmd, ValencyPoint::UnderArrow);
    mNextCommands[role] = newCmd;
}

void CmdCondition::insertSwitchStateToIf(Command* newCmd, ValencyPoint::Role role)
{
    Command* underArrow = nextCommand(ValencyPoint::UnderArrow);
    Command* right = nextCommand(ValencyPoint::Right);
    Command* down = nextCommand(ValencyPoint::Down);

    Command* cmd = underArrow;

    if (role == ValencyPoint::UnderArrow)
    {
        // recursively replace references to current "under arrow" command with new "under arrow" command
        if (down == underArrow)
        {
            mNextCommands[ValencyPoint::Down] = newCmd;
        }
        else
        {
            replaceReferences(underArrow, newCmd, down);
        }

        if (right == underArrow)
        {
            mNextCommands[ValencyPoint::Right] = newCmd;
        }
        else
        {
            replaceReferences(underArrow, newCmd, right);
        }
    }

    // old "under arrow" command now become "down" command for new command
    newCmd->replaceCommand(cmd, ValencyPoint::Down);
    mNextCommands[role] = newCmd;
}

//bool CmdQuestion::loadFromImpl(Command* other)
//{
//    CmdQuestion* otherQuestion = qobject_cast<CmdQuestion*>(other);
//    if (!otherQuestion)
//    {
//        LOG_ERROR(QString("Command type mismatch (not question)"));
//        return false;
//    }

//    mOperation = otherQuestion->operation();
//    mOrientation = otherQuestion->orientation();
//    mQuestionType = otherQuestion->questionType();
//    mOperands[Left] = otherQuestion->mOperands[Left];
//    mOperands[Right] = otherQuestion->mOperands[Right];

//    return true;
//}
