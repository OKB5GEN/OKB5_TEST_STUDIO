#include "Headers/logic/commands/cmd_sub_program.h"
#include "Headers/logger/Logger.h"
#include "Headers/file_reader.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logic/cyclogram_manager.h"

#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMetaEnum>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>

namespace
{
    static const QString SUBPROGRAM_PREFIX = "Sub";
    static const QString DELIMITER = ".";
}

CmdSubProgram::CmdSubProgram(QObject* parent):
    CmdAction(DRAKON::SUBPROGRAM, parent),
    mLoaded(false)
{
    mText = SUBPROGRAM_PREFIX;

    auto cyclogram = CyclogramManager::createCyclogram();

    mCyclogram = cyclogram;
    connect(cyclogram.data(), SIGNAL(finished(const QString&)), this, SLOT(onCyclogramFinished(const QString&)));
    connect(cyclogram.data(), SIGNAL(commandStarted(Command*)), this, SIGNAL(commandStarted(Command*)));
    connect(cyclogram.data(), SIGNAL(commandFinished(Command*)), this, SIGNAL(commandFinished(Command*)));

    updateText();
}

bool CmdSubProgram::load()
{
    setLoaded(false);

    auto cyclogram = mCyclogram.lock();
    cyclogram->setSystemState(mSystemState);

    if (mFilePath.isEmpty()) // no file link is normal
    {
        setLoaded(true);
        return true;
    }

    CyclogramManager::removeCyclogram(cyclogram);

    QString fileName = Cyclogram::defaultStorePath() + mFilePath;
    bool ok = false;
    cyclogram = CyclogramManager::createCyclogram(fileName, &ok);
    cyclogram->setSystemState(mSystemState);
    connect(cyclogram.data(), SIGNAL(finished(const QString&)), this, SLOT(onCyclogramFinished(const QString&)));
    connect(cyclogram.data(), SIGNAL(commandStarted(Command*)), this, SIGNAL(commandStarted(Command*)));
    connect(cyclogram.data(), SIGNAL(commandFinished(Command*)), this, SIGNAL(commandFinished(Command*)));

    mCyclogram = cyclogram;

    if (!ok)
    {
        return false;
    }

    setLoaded(true);
    return true;
}

QSharedPointer<Cyclogram> CmdSubProgram::cyclogram()
{
    return mCyclogram.lock();
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
        mErrorText = tr("Subprogram not loaded");
        emit criticalError(this);
        return;
    }

    // Set cyclogram variables current values according to input parameter mapping
    auto cyclogram = mCyclogram.lock();
    VariableController* vc = cyclogram->variableController();
    QMap<QString, qreal> variables;

    for (auto it = vc->variablesData().begin(); it != vc->variablesData().end(); ++it)
    {
        qreal value = 0;
        QVariant valueVariant = mInputParams.value(subprogramPrefix() + it.key());

        if (valueVariant.type() == QVariant::String)
        {
            value = mVarCtrl->currentValue(valueVariant.toString());
        }
        else if (valueVariant.type() == QVariant::Double)
        {
            value = valueVariant.toDouble();
        }

        variables[it.key()] = value;
        vc->setCurrentValue(it.key(), value);
    }

    mVarCtrl->startSubprogram(mText, variables);

    cyclogram->run();
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
    bool isOK = true;

    if (!mFilePath.isEmpty())
    {
        QString fileName = Cyclogram::defaultStorePath() + mFilePath;
        QFileInfo fileInfo(fileName);
        isOK = fileInfo.exists();
    }

    if (hasError())
    {
        setErrorStatus(!isOK);
    }
    else if (!isOK)
    {
        setErrorStatus(true);
    }

    emit textChanged(mText);

    // update system state if it is not set
    auto cyclogram = mCyclogram.lock();
    if (cyclogram && cyclogram->systemState() != mSystemState)
    {
        cyclogram->setSystemState(mSystemState);
    }
}

void CmdSubProgram::onNameChanged(const QString& newName, const QString& oldName)
{
    // input parameters update
    for (auto it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (it.value().type() == QVariant::String && it.value().toString() == oldName)
        {
            *it = newName;
        }
    }

    // output parameters update
    QVariant oldValue;
    for (auto it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.key() == oldName)
        {
            if (it.value() == QVariant(oldName))
            {
                oldValue = newName;
            }
            else
            {
                oldValue = it.value(); // store old value mapping
            }
        }

        if (it.value().type() == QVariant::String && it.value().toString() == oldName)
        {
            *it = newName;
        }
    }

    mOutputParams.remove(oldName); // remove old
    mOutputParams[newName] = oldValue; // add new
    updateText();
}

void CmdSubProgram::onVariableRemoved(const QString& name)
{
    auto cyclogram = mCyclogram.lock();

    // input parameters update
    for (auto it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (it.value().type() == QVariant::String && it.value().toString() == name)
        {
            QStringList tokens = it.key().split(DELIMITER);
            if (tokens.size() != 2)
            {
                LOG_ERROR(QString("Invalid variable name '%1'").arg(it.key()));
                continue;
            }

            qreal value = cyclogram->variableController()->initialValue(tokens.at(1));
            LOG_WARNING(QString("Subprogram '%1' input link to variable '%2' is corrupted due to '%3' variable deletion. Replaced by initial value: %4")
                        .arg(mText)
                        .arg(it.key())
                        .arg(name)
                        .arg(value));

            *it = QVariant(value);
        }
    }

    // output parameters update
    for (auto it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.key() != name && it.value().type() == QVariant::String && it.value().toString() == name)
        {
            LOG_WARNING(QString("Subprogram '%1' output link to variable '%2' is corrupted due to '%3' variable deletion. Replaced by '%2' variable itself")
                        .arg(mText)
                        .arg(it.key())
                        .arg(name));

            *it = it.key();
        }
    }

    mOutputParams.remove(name); // remove value from mapping table
    updateText();
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
                            else //if (metaType == QMetaType::Double)
                            {
                                mInputParams[name] = attributes.value("value").toDouble();
                            }
//                            else
//                            {
//                                LOG_ERROR(QString("Unexpected input param '%1' type %2").arg(name).arg(metaType));
//                                mInputParams[name] = QVariant();
//                            }
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
                            else// if (metaType == QMetaType::Double)
                            {
                                mOutputParams[name] = attributes.value("value").toDouble();
                            }
//                            else
//                            {
//                                LOG_ERROR(QString("Unexpected output param '%1' type %2").arg(name).arg(metaType));
//                                mOutputParams[name] = QVariant();
//                            }
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
    auto cyclogram = mCyclogram.lock();
    cyclogram->stop();
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

    // Set calling cyclogram variables current values according to output parameter mapping
    auto cyclogram = mCyclogram.lock();
    VariableController* vc = cyclogram->variableController();

    // add subprogram data timeline to calling cyclogram
    mVarCtrl->addDataTimeline(vc->dataTimeline());
    QMap<QString, qreal> variables;

    for (auto it = mVarCtrl->variablesData().begin(); it != mVarCtrl->variablesData().end(); ++it)
    {
        qreal value = 0;
        QVariant valueVariant = mOutputParams.value(it.key());

        if (valueVariant.type() == QVariant::String)
        {
            QString variableName = valueVariant.toString();

            if (mVarCtrl->isVariableExist(variableName)) // set to own variable value
            {
                value = mVarCtrl->currentValue(variableName);
            }
            else // set to subprogram variable value
            {
                QStringList tokens = variableName.split(DELIMITER);
                if (tokens.size() == 2)
                {
                    value = vc->currentValue(tokens.at(1));
                }
                else
                {
                    LOG_ERROR(QString("Invalid variable name '%1'").arg(variableName));
                }
            }
        }
        else if (valueVariant.type() == QVariant::Double)
        {
            value = valueVariant.toDouble();
        }

        variables[it.key()] = value;
        mVarCtrl->setCurrentValue(it.key(), value);
    }

    mVarCtrl->endSubprogram(mText, variables);

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

QString CmdSubProgram::subprogramPrefix() const
{
    return (SUBPROGRAM_PREFIX + DELIMITER);
}

void CmdSubProgram::restart()
{
    mVarCtrl->restart();

    auto cyclogram = mCyclogram.lock();
    // clear all subprograms variables data
    for (auto it = cyclogram->commands().begin(); it != cyclogram->commands().end(); ++it)
    {
        if ((*it)->type() == DRAKON::SUBPROGRAM)
        {
            CmdSubProgram* subprogram = qobject_cast<CmdSubProgram*>(*it);
            subprogram->restart();
        }
    }
}

void CmdSubProgram::setLoaded(bool loaded)
{
    mLoaded = loaded;
}
