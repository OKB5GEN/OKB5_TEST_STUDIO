#include "Headers/file_reader.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/module_commands.h"
#include "Headers/shape_types.h"
#include "Headers/version.h"
#include "Headers/app_settings.h"
#include "Headers/logic/command.h"
#include "Headers/logic/commands/cmd_question.h"
#include "Headers/logic/commands/cmd_title.h"
#include "Headers/gui/cyclogram/valency_point.h"
#include "Headers/logger/Logger.h"

#include "Headers/gui/editor_window.h"

#include <QMetaEnum>
#include <QMessageBox>
#include <QFile>

FileReader::FileReader(QSharedPointer<Cyclogram> cyclogram)
    : mCyclogram(cyclogram)
{
}

bool FileReader::read(QIODevice *device)
{
    mXML.setDevice(device);

    if (mXML.readNextStartElement())
    {
        if (mXML.name() == "cyclogram")
        {
            Version fileVersion(mXML.attributes().value("version").toString());
            const Version& appVersion = AppSettings::instance().version();

            // Old app version can not read new files (absolutely incompatible file formats)
            if (appVersion.major() >= fileVersion.major())
            {
                // 1. Files with equal major version but different minor versions can be read in both directions,
                // but in case of appVer < fileVer some functionality will be inaccesiible and will be lost in case of file save.
                // Warn user about this.
                // 2. It can happen if we need to do application rollback from one minor version to another.
                // 3. Patch version must not affect on file format, its just hotfixes.
                // If file format change requred for hotfix, minor and/or major versions have to be increased
                if (appVersion.major() == fileVersion.major() && appVersion.minor() < fileVersion.minor())
                {
//                    QMessageBox::warning(EditorWindow::instance(),
//                                         QObject::tr("File version mismatch"),
//                                         QObject::tr("Application version (%1) is lower than file version (%2).\n Some file data can be lost if you change and save this file.\n Application update is recommended")
//                                         .arg(appVersion.toString())
//                                         .arg(fileVersion.toString()));
                    LOG_WARNING(QString("Application version (%1) is lower than file version (%2).\n Some file data can be lost if you change and save this file.\n Application update is recommended")
                                                                         .arg(appVersion.toString())
                                                                         .arg(fileVersion.toString()));
                }

                readCyclogram(fileVersion);
            }
            else
            {
                mXML.raiseError(QObject::tr("Incompatible cyclogram file version %1. Current application version is %2.").arg(fileVersion.toString()).arg(appVersion.toString()));
            }
        }
        else
        {
            mXML.raiseError(QObject::tr("The file is not an cyclogram file."));
        }
    }

    return !mXML.error();
}

QString FileReader::errorString() const
{
    return QObject::tr("%1\nLine %2, column %3")
            .arg(mXML.errorString())
            .arg(mXML.lineNumber())
            .arg(mXML.columnNumber());
}

void FileReader::readCyclogram(const Version& fileVersion)
{
    while (!mXML.atEnd() && !mXML.hasError())
    {
        QXmlStreamReader::TokenType token = mXML.readNext();

        if (token == QXmlStreamReader::StartElement)
        {
            QString name = mXML.name().toString();

            if (name == "settings")
            {
                readSettings(fileVersion);
            }
            else if (name == "variables")
            {
                readVariables(fileVersion);
            }
            else if (name == "commands")
            {
                readCommands(fileVersion);
            }
            else if (name == "tree")
            {
                readCommandsLinks(fileVersion);
            }
        }
    }
}

void FileReader::readSettings(const Version& fileVersion)
{
    auto cyclogram = mCyclogram.lock();

    while (!(mXML.tokenType() == QXmlStreamReader::EndElement && mXML.name() == "settings"))
    {
        if (mXML.tokenType() == QXmlStreamReader::StartElement && mXML.name() == "setting")
        {
            QXmlStreamAttributes attributes = mXML.attributes();
            QString name;
            QString value;

            if (attributes.hasAttribute("name"))
            {
                name = attributes.value("name").toString();
            }

            if (attributes.hasAttribute("value"))
            {
                value = attributes.value("value").toString();
            }

            if (!name.isEmpty())
            {
                cyclogram->setSetting(name, value, false);
            }
        }

        mXML.readNext();
    }
}

void FileReader::readVariables(const Version& fileVersion)
{
    auto cyclogram = mCyclogram.lock();

    while (!(mXML.tokenType() == QXmlStreamReader::EndElement && mXML.name() == "variables"))
    {
        if (mXML.tokenType() == QXmlStreamReader::StartElement && mXML.name() == "variable")
        {
            QXmlStreamAttributes attributes = mXML.attributes();
            QString name;
            QString value;
            QString desc;

            if (attributes.hasAttribute("name"))
            {
                name = attributes.value("name").toString();
            }

            if (attributes.hasAttribute("value"))
            {
                value = attributes.value("value").toString();
            }

            if (attributes.hasAttribute("desc"))
            {
                desc = attributes.value("desc").toString();
            }

            VariableController* varCtrl = cyclogram->variableController();
            varCtrl->addVariable(name, value.toDouble());
            varCtrl->setDescription(name, desc);
        }

        mXML.readNext();
    }
}

void FileReader::readCommands(const Version& fileVersion)
{
    QMetaEnum commandTypes = QMetaEnum::fromType<DRAKON::IconType>();
    auto cyclogram = mCyclogram.lock();

    // read file, create commands and create links data
    while (!(mXML.tokenType() == QXmlStreamReader::EndElement && mXML.name() == "commands"))
    {
        if (mXML.tokenType() == QXmlStreamReader::StartElement && mXML.name() == "command")
        {
            QXmlStreamAttributes attributes = mXML.attributes();
            QString str;
            DRAKON::IconType type;

            if (attributes.hasAttribute("type"))
            {
                str = attributes.value("type").toString();
                type = DRAKON::IconType(commandTypes.keyToValue(qPrintable(str)));
            }

            Command* command = cyclogram->createCommand(type);
            if (command)
            {
                command->read(&mXML, fileVersion); // read command custom data
                mCommands[command->id()] = command;

                if (command->type() == DRAKON::TERMINATOR)
                {
                    CmdTitle* titleCmd = qobject_cast<CmdTitle*>(command);
                    if (titleCmd->titleType() == CmdTitle::BEGIN)
                    {
                        cyclogram->setFirst(command);
                    }
                    else
                    {
                        cyclogram->setLast(command);
                    }
                }
            }
        }

        mXML.readNext();
    }
}

void FileReader::readCommandsLinks(const Version& fileVersion)
{
    if (mCommands.empty()) //TODO validate file reading (temporary create default cyclogram)
    {
        LOG_WARNING(QString("File not loaded creating default cyclogram"));
        auto cyclogram = mCyclogram.lock();
        cyclogram->createDefault();
        return;
    }

    QMap<qint64, QList<qint64>> commandLinks;

    while (!(mXML.tokenType() == QXmlStreamReader::EndElement && mXML.name() == "tree"))
    {
        if (mXML.tokenType() == QXmlStreamReader::StartElement && mXML.name() == "item")
        {
            QXmlStreamAttributes attributes = mXML.attributes();
            QList<qint64> nextCommmands = {-1, -1, -1};

            qint64 commandID = -1;
            if (attributes.hasAttribute("id"))
            {
                commandID = attributes.value("id").toLongLong();
            }

            qint64 id = -1;
            if (attributes.hasAttribute("down"))
            {
                id = attributes.value("down").toLongLong();
            }

            nextCommmands[ValencyPoint::Down] = id;

            id = -1;
            if (attributes.hasAttribute("right"))
            {
                id = attributes.value("right").toLongLong();
            }

            nextCommmands[ValencyPoint::Right] = id;

            id = -1;
            if (attributes.hasAttribute("under_arrow"))
            {
                id = attributes.value("under_arrow").toLongLong();
            }

            nextCommmands[ValencyPoint::UnderArrow] = id;

            if (commandID != -1)
            {
                commandLinks[commandID] = nextCommmands;
            }
        }

        mXML.readNext();
    }

    // create command links
    for (QMap<qint64, QList<qint64>>::const_iterator it = commandLinks.begin(); it != commandLinks.end(); ++it)
    {
        Command * parentCmd = mCommands.value(it.key(), Q_NULLPTR);
        if (!parentCmd)
        {
            continue;
        }

        for(int i = 0; i <= ValencyPoint::UnderArrow; ++i)
        {
            qint64 index = it.value()[i];
            Command* nextCmd = mCommands.value(index, Q_NULLPTR);
            parentCmd->replaceCommand(nextCmd, ValencyPoint::Role(i));
        }
    }
}

Version FileReader::fileVersion(const QString& fileName, bool* ok)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        if (ok)
        {
            *ok = false;
        }

        return Version();
    }

    QXmlStreamReader xml;
    xml.setDevice(&file);

    if (xml.readNextStartElement())
    {
        if (xml.name() == "cyclogram" && xml.attributes().hasAttribute("version"))
        {
            if (ok)
            {
                *ok = true;
            }

            return Version(xml.attributes().value("version").toString());
        }
    }

    if (ok)
    {
        *ok = false;
    }

    return Version();
}
