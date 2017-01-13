#include "Headers/file_reader.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"

//#include <QtWidgets>

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
    Q_ASSERT(mXML.isStartElement() && mXML.name() == "commands");

    mCyclogram->createDefault(); //TODO temporary
}
