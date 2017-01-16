#include "Headers/file_reader.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/module_commands.h"
#include "Headers/shape_types.h"

#include "Headers/logic/command.h"
#include "Headers/logic/commands/cmd_question.h"
#include "Headers/gui/cyclogram/valency_point.h"

#include <QMetaEnum>

FileReader::FileReader(Cyclogram *cyclogram)
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
    //Q_ASSERT(mXML.isStartElement() && mXML.name() == "cyclogram");

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


    /*
    while (mXML.readNextStartElement())
    {
        QString name = mXML.name().toString();

        if (mXML.name() == "variables")
        {
            readVariables();
        }
        else if (mXML.name() == "commands")
        {
            readCommands();
        }
        else
        {
            mXML.skipCurrentElement();
        }
    }
    */
}

void FileReader::readVariables()
{
    while (!(mXML.tokenType() == QXmlStreamReader::EndElement && mXML.name() == "variables"))
    {
        if (mXML.tokenType() == QXmlStreamReader::StartElement && mXML.name() == "variable")
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

            VariableController* varCtrl = mCyclogram->variableController();
            varCtrl->addVariable(name, value.toDouble());
        }

        mXML.readNext();
    }

    /*Q_ASSERT(mXML.isStartElement() && mXML.name() == "variables");

    while (mXML.readNextStartElement())
    {
        QString name = mXML.name().toString();

        if (mXML.name() == "variable")
        {
            QString name = mXML.attributes().value("name").toString();
            QString value = mXML.attributes().value("value").toString();

            VariableController* varCtrl = mCyclogram->variableController();
            varCtrl->addVariable(name, value.toDouble());
        }
        else
        {
            mXML.skipCurrentElement();
        }
    }*/
}

void FileReader::readCommands()
{
    //Q_ASSERT(mXML.isStartElement() && mXML.name() == "commands");

    QMetaEnum commandTypes = QMetaEnum::fromType<DRAKON::IconType>();
    QMetaEnum questionTypes = QMetaEnum::fromType<CmdQuestion::QuestionType>();

    //mCyclogram->clear();

    // read file, create commands and create links data
    while (!(mXML.tokenType() == QXmlStreamReader::EndElement && mXML.name() == "commands"))
    {
        if (mXML.tokenType() == QXmlStreamReader::StartElement && mXML.name() == "command")
        {
            QXmlStreamAttributes attributes = mXML.attributes();
            QString str;
            qint64 commandID;
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

            Command* command = mCyclogram->createCommand(type, questionType);
            if (command)
            {
                command->read(&mXML); // read command custom data
                mCommands[command->id()] = command;
            }
        }

        mXML.readNext();
    }
}

void FileReader::readCommandsLinks()
{
    if (mCommands.empty()) //TODO validate file reading (temporary create default cyclogram)
    {
        qDebug("File not loaded create default cyclogram");
        mCyclogram->createDefault();
        return;
    }

    QMap<qint64, QList<qint64>> commandLinks;

    while (!(mXML.tokenType() == QXmlStreamReader::EndElement && mXML.name() == "tree"))
    {
        if (mXML.tokenType() == QXmlStreamReader::StartElement && mXML.name() == "item")
        {
            QXmlStreamAttributes attributes = mXML.attributes();
            QList<qint64> nextCommmands;

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
