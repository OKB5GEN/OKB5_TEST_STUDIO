#include "Headers/logic/commands/cmd_sub_program.h"
#include "Headers/logger/Logger.h"
#include "Headers/file_reader.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logic/cyclogram_manager.h"
#include "Headers/app_settings.h"
#include "Headers/file_writer.h"

#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMetaEnum>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QFileSystemWatcher>

namespace
{
    static const QString DEFAULT_TEXT = "Subprogram";
}

CmdSubProgram::CmdSubProgram(QObject* parent):
    Command(DRAKON::SUBPROGRAM, 1, parent),
    mLoaded(false),
    mNeedHashUpdate(false)
{
    mFileWatcher = new QFileSystemWatcher(this);

    mText = DEFAULT_TEXT;

    auto cyclogram = CyclogramManager::createCyclogram();

    setCyclogram(cyclogram);
    updateText();
}

CmdSubProgram::~CmdSubProgram()
{
    CyclogramManager::removeCyclogram(mCyclogram.lock());
}

bool CmdSubProgram::load()
{
    setLoaded(false);

    auto cyclogram = mCyclogram.lock();
    cyclogram->setSystemState(mSystemState);

    if (mFilePath.isEmpty())
    {
        return false;
    }

    CyclogramManager::removeCyclogram(cyclogram);

    QString fileName = Cyclogram::defaultStorePath() + mFilePath;
    bool ok = false;
    cyclogram = CyclogramManager::createCyclogram(fileName, &ok);
    cyclogram->setSystemState(mSystemState);

    setCyclogram(cyclogram);

    if (!ok)
    {
        return false;
    }

    mFileHash = FileReader::fileHash(fileName);

    if (!mFileWatcher->files().empty())
    {
        mFileWatcher->removePaths(mFileWatcher->files());
    }

    Qt::ConnectionType connection = Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection);
    connect(mFileWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(reloadCyclogram()), connection);
    mFileWatcher->addPath(fileName);

    if (mText == DEFAULT_TEXT)
    {
        mText = cyclogram->setting(Cyclogram::SETTING_DEFAULT_NAME).toString();
    }

    if (mText.isEmpty())
    {
        mText = DEFAULT_TEXT;
    }

    setLoaded(true);
    return true;
}

void CmdSubProgram::setCyclogram(QSharedPointer<Cyclogram> cyclogram)
{
    if (mCyclogram.lock())
    {
        mCyclogram.data()->disconnect(this);
    }

    connect(cyclogram.data(), SIGNAL(finished(const QString&)), this, SLOT(onCyclogramFinished(const QString&)));
    connect(cyclogram.data(), SIGNAL(commandStarted(Command*)), this, SIGNAL(commandStarted(Command*)));
    connect(cyclogram.data(), SIGNAL(commandFinished(Command*)), this, SIGNAL(commandFinished(Command*)));
    connect(cyclogram.data(), SIGNAL(modified()), this, SLOT(onCyclogramModified()));

    connect(cyclogram->variableController(), SIGNAL(variableAdded(const QString&, qreal)), this, SLOT(onInnerVariableAdded(const QString&, qreal)));
    connect(cyclogram->variableController(), SIGNAL(variableRemoved(const QString&)), this, SLOT(onInnerVariableRemoved(const QString&)));
    connect(cyclogram->variableController(), SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(onInnerVariableNameChanged(const QString&, const QString&)));

    mCyclogram = cyclogram;
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
        QVariant valueVariant = mInputParams.value(it.key());

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

void CmdSubProgram::setFilePath(const QString& filePath, bool reload)
{
    if (mFilePath == filePath)
    {
        if (reload && !loaded())
        {
            load();
            updateText();
        }

        return;
    }

    if (!mFileWatcher->files().empty())
    {
        mFileWatcher->removePaths(mFileWatcher->files());
    }

    mFilePath = filePath;

    if (reload)
    {
        load();
        updateText();
    }
}

bool CmdSubProgram::loaded() const
{
    return mLoaded;
}

void CmdSubProgram::setName(const QString& name)
{
    QString nameBefore = mText;
    mText = name;
    if (nameBefore != mText)
    {
        updateText();
    }
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
    else
    {
        isOK = false;
    }

    if (hasError())
    {
        setErrorStatus(!isOK);
    }
    else if (!isOK)
    {
        setErrorStatus(true);
    }

    emit dataChanged(mText);

    // update system state if it is not set
    auto cyclogram = mCyclogram.lock();
    if (cyclogram && cyclogram->systemState() != mSystemState)
    {
        cyclogram->setSystemState(mSystemState);
    }
}

void CmdSubProgram::onInnerVariableNameChanged(const QString& newName, const QString& oldName)
{
    // input parameters update
    for (auto it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (it.key() == oldName)
        {
            mInputParams[newName] = it.value();
            mInputParams.remove(oldName);
            break;
        }
    }

    // output parameters update
    for (auto it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.value().type() == QVariant::String && it.value().toString() == oldName)
        {
            *it = newName;
        }
    }

    updateText();
}

void CmdSubProgram::onInnerVariableRemoved(const QString& name)
{
    // input parameters update
    mInputParams.remove(name);

    // output parameters update
    for (auto it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.value().type() == QVariant::String && it.value().toString() == name)
        {
            LOG_WARNING(QString("Subprogram '%1' output link to variable '%2' is corrupted due to '%3' variable deletion. Removing link")
                        .arg(mText)
                        .arg(it.key())
                        .arg(name));

            *it = QVariant();
        }
    }

    updateText();
}

void CmdSubProgram::onInnerVariableAdded(const QString& name, qreal value)
{
    // input parameters update
    mInputParams[name] = value;

    // output parameters update (not needed)
    updateText();
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
            qreal value = cyclogram->variableController()->initialValue(it.key());
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

    saveFileIfNotExist();

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

void CmdSubProgram::readCustomAttributes(QXmlStreamReader* reader, const Version& fileVersion)
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
                            else
                            {
                                mInputParams[name] = attributes.value("value").toDouble();
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
                            else
                            {
                                mOutputParams[name] = attributes.value("value").toDouble();
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
    VariableController* subprogramVC = cyclogram->variableController();

    // add subprogram data timeline to calling cyclogram
    mVarCtrl->addDataTimeline(subprogramVC->dataTimeline());
    QMap<QString, qreal> variables;

    for (auto it = mVarCtrl->variablesData().begin(); it != mVarCtrl->variablesData().end(); ++it)
    {
        qreal value = 0;
        QVariant valueVariant = mOutputParams.value(it.key());

        if (!valueVariant.isValid())
        {
            continue;
        }

        if (valueVariant.type() == QVariant::String)
        {
            QString variableName = valueVariant.toString();

            if (variableName.isEmpty())
            {
                continue;
            }

            value = subprogramVC->currentValue(variableName);
        }
        else if (valueVariant.type() == QVariant::Double)
        {
            value = valueVariant.toDouble();
        }

        variables[it.key()] = value;
        mVarCtrl->setCurrentValue(it.key(), value);
    }

    mVarCtrl->endSubprogram(mText, variables);

    emit finished(nextCommand()); // command succesfully executed
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
    QMap<QString, QVariant> inBefore = mInputParams;
    QMap<QString, QVariant> outBefore = mOutputParams;
    mInputParams = in;
    mOutputParams = out;

    bool isDataChanged = false;

    for (auto it = inBefore.begin(); it != inBefore.end(); ++it)
    {
        auto iter = mInputParams.find(it.key());
        if (iter != mInputParams.end())
        {
            if (iter.value() != it.value())
            {
                isDataChanged = true;
                break;
            }
        }
    }

    if (!isDataChanged)
    {
        for (auto it = outBefore.begin(); it != outBefore.end(); ++it)
        {
            auto iter = mOutputParams.find(it.key());
            if (iter != mOutputParams.end())
            {
                if (iter.value() != it.value())
                {
                    isDataChanged = true;
                    break;
                }
            }
        }
    }

    if (isDataChanged)
    {
        updateText();
    }
}

void CmdSubProgram::restart()
{
    mVarCtrl->restart();

    auto cyclogram = mCyclogram.lock();
    cyclogram->setState(Cyclogram::PENDING_FOR_START);

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

bool CmdSubProgram::loadFromImpl(Command* other)
{
    CmdSubProgram* otherSubprogram = qobject_cast<CmdSubProgram*>(other);
    if (!otherSubprogram)
    {
        LOG_ERROR(QString("Command type mismatch (not subprogram)"));
        return false;
    }

    mText = other->text();
    setFilePath(otherSubprogram->filePath());
    mInputParams = otherSubprogram->inputParams();
    mOutputParams = otherSubprogram->outputParams();

    return true;
}

void CmdSubProgram::onCyclogramModified()
{
    updateText();
}

void CmdSubProgram::generateFileName()
{
    QString fileName = "Sub_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + AppSettings::extension();
    setFilePath(fileName, false);
}

void CmdSubProgram::saveFileIfNotExist()
{
    if (mFilePath.isEmpty())
    {
        LOG_ERROR(QString("Subprogram file path is empty!"));
        return;
    }

    QString fileName = Cyclogram::defaultStorePath() + mFilePath;
    if (QFileInfo(fileName).exists())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        LOG_ERROR(QString("Cannot write file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    FileWriter writer(mCyclogram.lock());
    if (!writer.writeFile(&file))
    {
        LOG_ERROR(QString("File '%1' not saved!").arg(fileName));
        return;
    }

    updateText();
}

void CmdSubProgram::beforeSave()
{
    mNeedHashUpdate = true;
}

void CmdSubProgram::reloadCyclogram()
{
    QString hash = FileReader::fileHash(Cyclogram::defaultStorePath() + mFilePath);

    if (mNeedHashUpdate)
    {
        mFileHash = hash;
        mNeedHashUpdate = false;
        return;
    }

    if (mFileHash == hash)
    {
        return;
    }

    mFileHash = hash;

    LOG_WARNING(QString("Subprogram file '%1' changed outside the editor.").arg(mFilePath));

    auto cyclogram = mCyclogram.lock();
    if (cyclogram->isModified())
    {
        generateFileName();
        LOG_WARNING(QString("Subprogram has local unsaved modifications. New file name is '%1'").arg(mFilePath));
        return;
    }

    LOG_WARNING(QString("Reloading file '%1'").arg(mFilePath));

    QString filePath = mFilePath;
    setFilePath("", false);
    setFilePath(filePath);

    cyclogram = mCyclogram.lock();
    cyclogram->setModified(false, true, false);

    emit cyclogramChanged();
}
