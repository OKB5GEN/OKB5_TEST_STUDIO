#include "Headers/logic/commands/cmd_sub_program.h"
#include "Headers/logger/Logger.h"
#include "Headers/file_reader.h"
#include "Headers/logic/cyclogram.h"


#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMetaEnum>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>


/* Команда-подпрограмма. Мысли вслух о реализации.
 *
 * 1. Данная команда - это по сути один файл циклограммы, сведенный в один квадратик
 * 2. Выполнение данной команды - это выполнение циклограммы
 * 3. Есть правда потенциальная опасность зацикливания циклограммы, когда подпрограмма вызывает другую подпрограмму, а та вызывает копию вызывающей (но это похеру)
 * 4. ПЕРЕМЕННЫЕ ПОДПРОГРАМЫ ИЗ ВЫЗЫВАЮЩЕЙ ПРОГРАММЫ НЕ ВИДНЫ!
 * 5. При редактировании команды мы заполняем две таблицы: "входные параметры" и "выходные параметры"
 * 6. "Входные параметры" - это переменные циклограммы-подпрограммы, которые могут быть проинициализированы либо ТЕКУЩИМ ЗНАЧЕНИЕМ переменной вызывающей циклограммы, либо числом.
 *    По умолчанию все переменные подпрограммы проинициализированы числами-начальными значениями из файла циклограммы-подпрограммы.
 * 7. "Выходные параметры" - это перемнные вызывающей циклограммы, которые могут быть изменены по результатам выполнения подпрограммы.
 *    По умолчанию выходные параметры равны сами себе (подпрограмма по умолчанию ничего не меняет), но могут быть поменяны на значение переменной подпрограммы (на число не могут)
 * 8. Внутри команды мы храним и копию циклограммы-подпрограммы и ссылку на файл:
 *    - Если копия и файл не сошлись, то спрашиваем что юзать
 *    - Если файл удалили/переместили/переименовали - всегда есть локальная копия внутри циклограммы
 *    - Если файл поменяли, то его можно перезагрузить (при открытии файла циклограммы и создании команды-подпрограммы мы спрашиваем чо делать)
 * 9. Так же мы можем хранить только ссылку на файл и маппинг входных-выходных параметров подпрограммы
 *
 *
 *
 * Гибридный вариант (сохраняем и ссылку и копию):
*/

CmdSubProgram::CmdSubProgram(QObject* parent):
    CmdAction(DRAKON::SUBPROGRAM, parent),
    mCyclogram(Q_NULLPTR)
{
    mCyclogram = new Cyclogram(this);
    mCyclogram->setMainCyclogram(false);
    connect(mCyclogram, SIGNAL(finished(const QString&)), this, SLOT(onCyclogramFinished(const QString&)));
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
    // Load cyclogram
    mCyclogram->clear();

    QFile file(mFilePath);
    FileReader reader(mCyclogram);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        LOG_ERROR(QString("Cannot read file %1: %2").arg(QDir::toNativeSeparators(mFilePath), file.errorString()));
        emit criticalError(this);
        return;
    }

    if (!reader.read(&file))
    {
        LOG_ERROR(QString("Parse error in file %1: %2").arg(QDir::toNativeSeparators(mFilePath), reader.errorString()));
        emit criticalError(this);
        return;
    }

    mCyclogram->run();



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

//    finish();
}

void CmdSubProgram::setFilePath(const QString& filePath)
{
    mFilePath = filePath;

    if (!QFileInfo(mFilePath).exists())
    {
        LOG_ERROR(QString("Subprogram command error. File '%1' does not exist").arg(mFilePath));
    }

    updateText();
}

void CmdSubProgram::updateText()
{
    mText = "ERROR";
    QFileInfo fileInfo(mFilePath);

    bool isOK = (!mFilePath.isEmpty() && fileInfo.exists());

    if (hasError())
    {
        setErrorStatus(!isOK);
    }
    else if (!isOK)
    {
        setErrorStatus(true);
    }

    if (isOK)
    {
        mText = fileInfo.fileName();
    }

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
    writer->writeAttribute("file", mFilePath);

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
    QXmlStreamAttributes attributes = reader->attributes();
    if (attributes.hasAttribute("file"))
    {
        mFilePath = attributes.value("file").toString();
    }

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

    updateText();
}

const QString& CmdSubProgram::filePath() const
{
    return mFilePath;
}

void CmdSubProgram::stop()
{
    mCyclogram->stop();
}

void CmdSubProgram::pause()
{
    mCyclogram->pause();
}

void CmdSubProgram::resume()
{
    mCyclogram->resume();
}

void CmdSubProgram::onCyclogramFinished(const QString& error)
{
    if (!error.isEmpty())
    {
        mErrorText = QString("Subprogram finished with error: %1").arg(error);
        emit criticalError(this);
        return;
    }

    finish();
}
