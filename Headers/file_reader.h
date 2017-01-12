#ifndef FILE_READER_H
#define FILE_READER_H

#include <QXmlStreamReader>

class Cyclogram;

class FileReader
{
public:
    FileReader(Cyclogram* cyclogram);

    bool read(QIODevice* device);
    QString errorString() const;

private:
    //void readXBEL();
    //void readTitle(QTreeWidgetItem *item);
    //void readSeparator(QTreeWidgetItem *item);
    //void readFolder(QTreeWidgetItem *item);
    //void readBookmark(QTreeWidgetItem *item);

    //QTreeWidgetItem *createChildItem(QTreeWidgetItem *item);

    Cyclogram* mCyclogram;
    QXmlStreamReader mXML;
};

#endif // FILE_READER_H
