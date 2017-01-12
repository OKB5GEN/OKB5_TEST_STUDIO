#include "Headers/file_reader.h"
#include "Headers/logic/cyclogram.h"

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
        //if (mXML.name() == "xbel" && xml.attributes().value("version") == "1.0")
        //{
        //    readXBEL();
        //}
        //else
        //{
        //    mXML.raiseError(QObject::tr("The file is not an XBEL version 1.0 file."));
        //}
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

/*

void FileReader::readXBEL()
{
    Q_ASSERT(mXML.isStartElement() && mXML.name() == "xbel");

    while (mXML.readNextStartElement())
    {
        if (mXML.name() == "folder")
        {
            readFolder(0);
        }
        else if (mXML.name() == "bookmark")
        {
            readBookmark(0);
        }
        else if (mXML.name() == "separator")
        {
            readSeparator(0);
        }
        else
        {
            mXML.skipCurrentElement();
        }
    }
}

void FileReader::readTitle(QTreeWidgetItem *item)
{
    Q_ASSERT(mXML.isStartElement() && mXML.name() == "title");

    QString title = mXML.readElementText();
    item->setText(0, title);
}

void FileReader::readSeparator(QTreeWidgetItem *item)
{
    Q_ASSERT(mXML.isStartElement() && mXML.name() == "separator");

    QTreeWidgetItem *separator = createChildItem(item);
    separator->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    separator->setText(0, QString(30, 0xB7));
    mXML.skipCurrentElement();
}

void FileReader::readFolder(QTreeWidgetItem *item)
{
    Q_ASSERT(mXML.isStartElement() && mXML.name() == "folder");

    QTreeWidgetItem *folder = createChildItem(item);
    bool folded = (mXML.attributes().value("folded") != "no");
    treeWidget->setItemExpanded(folder, !folded);

    while (mXML.readNextStartElement())
    {
        if (mXML.name() == "title")
        {
            readTitle(folder);
        }
        else if (mXML.name() == "folder")
        {
            readFolder(folder);
        }
        else if (mXML.name() == "bookmark")
        {
            readBookmark(folder);
        }
        else if (mXML.name() == "separator")
        {
            readSeparator(folder);
        }
        else
        {
            mXML.skipCurrentElement();
        }
    }
}

void FileReader::readBookmark(QTreeWidgetItem *item)
{
    Q_ASSERT(mXML.isStartElement() && mXML.name() == "bookmark");

    QTreeWidgetItem *bookmark = createChildItem(item);
    bookmark->setFlags(bookmark->flags() | Qt::ItemIsEditable);
    bookmark->setIcon(0, bookmarkIcon);
    bookmark->setText(0, QObject::tr("Unknown title"));
    bookmark->setText(1, xml.attributes().value("href").toString());

    while (mXML.readNextStartElement())
    {
        if (mXML.name() == "title")
        {
            readTitle(bookmark);
        }
        else
        {
            mXML.skipCurrentElement();
        }
    }
}

QTreeWidgetItem *FileReader::createChildItem(QTreeWidgetItem *item)
{
    QTreeWidgetItem *childItem;
    if (item)
    {
        childItem = new QTreeWidgetItem(item);
    }
    else
    {
        childItem = new QTreeWidgetItem(treeWidget);
    }

    childItem->setData(0, Qt::UserRole, mXML.name().toString());
    return childItem;
}
*/
