#ifndef FILE_READER_H
#define FILE_READER_H

#include <QXmlStreamReader>
#include <QMap>

class Cyclogram;
class Command;

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
    void readCommandsLinks();

    Cyclogram* mCyclogram;
    QMap<qint64, Command*> mCommands;
    QXmlStreamReader mXML;
};

#endif // FILE_READER_H
