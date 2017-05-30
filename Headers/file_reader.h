#ifndef FILE_READER_H
#define FILE_READER_H

#include <QXmlStreamReader>
#include <QMap>
#include <QSharedPointer>

class Cyclogram;
class Command;

class FileReader
{
public:
    FileReader(QSharedPointer<Cyclogram> cyclogram);

    bool read(QIODevice* device);
    QString errorString() const;

private:
    void readCyclogram();
    void readSettings();
    void readVariables();
    void readCommands();
    void readCommandsLinks();

    QWeakPointer<Cyclogram> mCyclogram;
    QMap<qint64, Command*> mCommands;
    QXmlStreamReader mXML;
};

#endif // FILE_READER_H
