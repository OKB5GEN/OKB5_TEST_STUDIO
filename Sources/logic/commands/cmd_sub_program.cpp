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
    mCyclogram(Q_NULLPTR),
    mLoaded(false)
{
    mText = "Sub";

    mCyclogram = new Cyclogram(this);
    mCyclogram->setMainCyclogram(false);
    connect(mCyclogram, SIGNAL(finished(const QString&)), this, SLOT(onCyclogramFinished(const QString&)));
    updateText();
}

bool CmdSubProgram::load()
{
    mLoaded = false;
    mCyclogram->clear();

    QFile file(mFilePath);
    FileReader reader(mCyclogram);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        LOG_ERROR(QString("Cannot read file %1: %2").arg(QDir::toNativeSeparators(mFilePath), file.errorString()));
        //emit criticalError(this);
        return false;
    }

    if (!reader.read(&file))
    {
        LOG_ERROR(QString("Parse error in file %1: %2").arg(QDir::toNativeSeparators(mFilePath), reader.errorString()));
        //emit criticalError(this);
        return false;
    }

    mLoaded = true;
    return true;
}

Cyclogram* CmdSubProgram::cyclogram() const
{
    return mCyclogram;
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
    if (!mLoaded)
    {
        emit criticalError(this);
        return;
    }

    int TODO; // set input params

    // Load cyclogram
    mCyclogram->run();
}

void CmdSubProgram::setFilePath(const QString& filePath)
{
    if (mFilePath == filePath)
    {
        return;
    }

    mFilePath = filePath;
    load();
    updateText();

//    if (!QFileInfo(mFilePath).exists())
//    {
//        LOG_ERROR(QString("Subprogram command error. File '%1' does not exist").arg(mFilePath));
//        updateText();
//        return;
//    }
}

bool CmdSubProgram::loaded() const
{
    return mLoaded;
}

void CmdSubProgram::setName(const QString& name)
{
    mText = name;
    updateText();
}

const QString& CmdSubProgram::name() const
{
    return mText;
}

void CmdSubProgram::updateText()
{
    QFileInfo fileInfo(mFilePath);

    bool isOK = (!mFilePath.isEmpty() && fileInfo.exists());

    //TODO no file name is OK (implement this case)

    if (hasError())
    {
        setErrorStatus(!isOK);
    }
    else if (!isOK)
    {
        setErrorStatus(true);
    }

    emit textChanged(mText);
}

void CmdSubProgram::onNameChanged(const QString& newName, const QString& oldName)
{
    int TODO;
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
    int TODO;
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
    writer->writeAttribute("name", mText);
    writer->writeAttribute("file", mFilePath);

    // input params
    writer->writeStartElement("input_params");
    for (auto it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        writer->writeStartElement("param");
        writer->writeAttribute("name", it.key());
        writer->writeAttribute("type", QString::number(int(it.value().type())));
        writer->writeAttribute("value", it.value().toString());
        writer->writeEndElement();
    }

    writer->writeEndElement();

    // output params
    writer->writeStartElement("output_params");
    for (auto it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        writer->writeStartElement("param");
        writer->writeAttribute("name", it.key());
        writer->writeAttribute("type", QString::number(int(it.value().type())));
        writer->writeAttribute("value", it.value().toString());
        writer->writeEndElement();
    }

    writer->writeEndElement();
}

void CmdSubProgram::readCustomAttributes(QXmlStreamReader* reader)
{
    QXmlStreamAttributes attributes = reader->attributes();
    if (attributes.hasAttribute("file"))
    {
        mFilePath = attributes.value("file").toString();
    }

    if (attributes.hasAttribute("name"))
    {
        mText = attributes.value("name").toString();
    }

    while (!(reader->tokenType() == QXmlStreamReader::EndElement && reader->name() == "command"))
    {
        if (reader->tokenType() == QXmlStreamReader::StartElement)
        {
            QString tokenName = reader->name().toString();

            if (tokenName == "input_params")
            {
                while (!(reader->tokenType() == QXmlStreamReader::EndElement && reader->name() == tokenName))
                {
                    if (reader->tokenType() == QXmlStreamReader::StartElement && reader->name() == "param")
                    {
                        attributes = reader->attributes();
                        QString name;

                        if (attributes.hasAttribute("name"))
                        {
                            name = attributes.value("name").toString();
                        }

                        int metaType = QMetaType::UnknownType;

                        if (attributes.hasAttribute("type"))
                        {
                            metaType = attributes.value("type").toInt();
                        }

                        if (attributes.hasAttribute("value"))
                        {
                            if (metaType == QMetaType::QString)
                            {
                                mInputParams[name] = attributes.value("value").toString();
                            }
                            else if (metaType == QMetaType::Double)
                            {
                                mInputParams[name] = attributes.value("value").toDouble();
                            }
                            else
                            {
                                LOG_ERROR(QString("Unexpected input param '%1' type %2").arg(name).arg(metaType));
                                mInputParams[name] = QVariant();
                            }
                        }
                    }

                    reader->readNext();
                }
            }
            else if (tokenName == "output_params")
            {
                while (!(reader->tokenType() == QXmlStreamReader::EndElement && reader->name() == tokenName))
                {
                    if (reader->tokenType() == QXmlStreamReader::StartElement && reader->name() == "param")
                    {
                        attributes = reader->attributes();
                        QString name;

                        if (attributes.hasAttribute("name"))
                        {
                            name = attributes.value("name").toString();
                        }

                        int metaType = QMetaType::UnknownType;

                        if (attributes.hasAttribute("type"))
                        {
                            metaType = attributes.value("type").toInt();
                        }

                        if (attributes.hasAttribute("value"))
                        {
                            if (metaType == QMetaType::QString)
                            {
                                mOutputParams[name] = attributes.value("value").toString();
                            }
                            else if (metaType == QMetaType::Double)
                            {
                                mOutputParams[name] = attributes.value("value").toDouble();
                            }
                            else
                            {
                                LOG_ERROR(QString("Unexpected output param '%1' type %2").arg(name).arg(metaType));
                                mOutputParams[name] = QVariant();
                            }
                        }
                    }

                    reader->readNext();
                }
            }
        }

        reader->readNext();
    }

    load();
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

#ifdef ENABLE_CYCLOGRAM_PAUSE
void CmdSubProgram::pause()
{
    mCyclogram->pause();
}

void CmdSubProgram::resume()
{
    mCyclogram->resume();
}
#endif

void CmdSubProgram::onCyclogramFinished(const QString& error)
{
    if (!error.isEmpty())
    {
        mErrorText = QString("Subprogram finished with error: %1").arg(error);
        emit criticalError(this);
        return;
    }

    int TODO; // write output params

    finish();
}

const QMap<QString, QVariant>& CmdSubProgram::inputParams() const
{
    return mInputParams;
}

const QMap<QString, QVariant>& CmdSubProgram::outputParams() const
{
    return mOutputParams;
}

void CmdSubProgram::setParams(const QMap<QString, QVariant>& in, const QMap<QString, QVariant>& out)
{
    mInputParams = in;
    mOutputParams = out;
    updateText();
}
