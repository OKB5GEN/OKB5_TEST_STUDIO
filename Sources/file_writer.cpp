#include "Headers/file_writer.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logic/command.h"

FileWriter::FileWriter(Cyclogram* cyclogram)
    : mCyclogram(cyclogram)
{
    mXML.setAutoFormatting(true);
}

bool FileWriter::writeFile(QIODevice *device)
{
    mXML.setDevice(device);

    mXML.writeStartDocument();
    mXML.writeDTD("<!DOCTYPE cyclogram>");
    mXML.writeStartElement("cyclogram");
    mXML.writeAttribute("version", "1.0");

    writeVariables();
    writeCommands();
    writeCommandTree();

    mXML.writeEndDocument();
    return true;
}

void FileWriter::writeVariables()
{
    VariableController* varCtrl = mCyclogram->variableController();

    mXML.writeStartElement("variables");

    for (auto it = varCtrl->variablesData().begin(); it != varCtrl->variablesData().end(); ++it)
    {
        mXML.writeStartElement("variable");
        mXML.writeAttribute("name", it.key());
        mXML.writeAttribute("value", QString::number(it.value().initialValue));
        mXML.writeAttribute("desc", it.value().description);
        mXML.writeEndElement();
    }

    mXML.writeEndElement();
}

void FileWriter::writeCommands()
{
    mXML.writeStartElement("commands");

    foreach (Command* command, mCyclogram->commands())
    {
        command->write(&mXML);
    }

    mXML.writeEndElement();
}

void FileWriter::writeCommandTree()
{
    mXML.writeStartElement("tree");

    foreach (Command* command, mCyclogram->commands())
    {
        mXML.writeStartElement("item");
        mXML.writeAttribute("id", QString::number(command->id()));

        Command* down = command->nextCommand(ValencyPoint::Down);
        Command* right = command->nextCommand(ValencyPoint::Right);
        Command* underArrow = command->nextCommand(ValencyPoint::UnderArrow);

        mXML.writeAttribute("down", down ? QString::number(down->id()) : "-1");
        mXML.writeAttribute("right", right ? QString::number(right->id()) : "-1");
        mXML.writeAttribute("under_arrow", underArrow ? QString::number(underArrow->id()) : "-1");
        mXML.writeEndElement();
    }

    mXML.writeEndElement();
}

