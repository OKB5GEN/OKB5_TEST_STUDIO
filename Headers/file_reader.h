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
    void readCyclogram();
    void readVariables();
    void readCommands();

    Cyclogram* mCyclogram;
    QXmlStreamReader mXML;
};

#endif // FILE_READER_H
