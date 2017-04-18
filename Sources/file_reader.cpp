#include "Headers/file_reader.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/module_commands.h"
#include "Headers/shape_types.h"

#include "Headers/logic/command.h"
#include "Headers/logic/commands/cmd_question.h"
#include "Headers/logic/commands/cmd_title.h"
#include "Headers/gui/cyclogram/valency_point.h"
#include "Headers/logger/Logger.h"

#include <QMetaEnum>

FileReader::FileReader(QSharedPointer<Cyclogram> cyclogram)
    : mCyclogram(cyclogram)
{
}

bool FileReader::read(QIODevice *device)
{
    mXML.setDevice(device);

    if (mXML.readNextStartElement())
    {
        if (mXML.name() == "cyclogram" && mXML.attributes().value("version") == "1.0")
        {
            readCyclogram();
        }
        else
        {
            mXML.raiseError(QObject::tr("The file is not an cyclogram version 1.0 file."));
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

void FileReader::readCyclogram()
{
    while (!mXML.atEnd() && !mXML.hasError())
    {
        QXmlStreamReader::TokenType token = mXML.readNext();

        if (token == QXmlStreamReader::StartElement)
        {
            QString name = mXML.name().toString();

            if (name == "variables")
            {
                readVariables();
            }
            else if (name == "commands")
            {
                readCommands();
            }
            else if (name == "tree")
            {
                readCommandsLinks();
            }
        }
    }
}

void FileReader::readVariables()
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

void FileReader::readCommands()
{
    QMetaEnum commandTypes = QMetaEnum::fromType<DRAKON::IconType>();
    QMetaEnum questionTypes = QMetaEnum::fromType<CmdQuestion::QuestionType>();
    auto cyclogram = mCyclogram.lock();

    // read file, create commands and create links data
    while (!(mXML.tokenType() == QXmlStreamReader::EndElement && mXML.name() == "commands"))
    {
        if (mXML.tokenType() == QXmlStreamReader::StartElement && mXML.name() == "command")
        {
            QXmlStreamAttributes attributes = mXML.attributes();
            QString str;
            //qint64 commandID;
            DRAKON::IconType type;
            CmdQuestion::QuestionType questionType;

            if (attributes.hasAttribute("type"))
            {
                str = attributes.value("type").toString();
                type = DRAKON::IconType(commandTypes.keyToValue(qPrintable(str)));
            }

            if (type == DRAKON::QUESTION && attributes.hasAttribute("cmd_type")) //TODO move to question command loading
            {
                str = attributes.value("cmd_type").toString();
                questionType = CmdQuestion::QuestionType(questionTypes.keyToValue(qPrintable(str)));
            }

            Command* command = cyclogram->createCommand(type, questionType);
            if (command)
            {
                command->read(&mXML); // read command custom data
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

void FileReader::readCommandsLinks()
{
    if (mCommands.empty()) //TODO validate file reading (temporary create default cyclogram)
    {
        LOG_WARNING(QString("File not loaded create default cyclogram"));
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
