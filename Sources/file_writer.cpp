#include "Headers/file_writer.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logic/command.h"

//#include <QtWidgets>

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

    mXML.writeEndDocument();
    return true;
}

void FileWriter::writeVariables()
{
    VariableController* varCtrl = mCyclogram->variableController();
    const QMap<QString, qreal>& values = varCtrl->variables(VariableController::Initial);

    mXML.writeStartElement("variables");

    for (QMap<QString, qreal>::const_iterator it = values.begin(); it != values.end(); ++it)
    {
        mXML.writeStartElement("variable");
        mXML.writeAttribute("name", it.key());
        mXML.writeAttribute("value", QString::number(it.value()));
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

/*
void FileWriter::writeItem(QTreeWidgetItem *item)
{
    QString tagName = item->data(0, Qt::UserRole).toString();
    if (tagName == "folder")
    {
        bool folded = !treeWidget->isItemExpanded(item);
        mXML.writeStartElement(tagName);
        mXML.writeAttribute("folded", folded ? "yes" : "no");
        mXML.writeTextElement("title", item->text(0));
        for (int i = 0; i < item->childCount(); ++i)
        {
            writeItem(item->child(i));
        }

        mXML.writeEndElement();
    }
    else if (tagName == "bookmark")
    {
        mXML.writeStartElement(tagName);
        if (!item->text(1).isEmpty())
        {
            mXML.writeAttribute("href", item->text(1));
        }

        mXML.writeTextElement("title", item->text(0));
        mXML.writeEndElement();
    }
    else if (tagName == "separator")
    {
        mXML.writeEmptyElement(tagName);
    }
}
*/
