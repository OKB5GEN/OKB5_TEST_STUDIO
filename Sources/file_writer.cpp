#include "Headers/file_writer.h"
#include "Headers/logic/cyclogram.h"

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
    //mXML.writeDTD("<!DOCTYPE xbel>");
    //mXML.writeStartElement("xbel");
    //mXML.writeAttribute("version", "1.0");
    //for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    //{
    //    writeItem(treeWidget->topLevelItem(i));
    //}

    mXML.writeEndDocument();
    return true;
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
