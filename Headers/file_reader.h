#ifndef FILE_READER_H
#define FILE_READER_H

#include <QXmlStreamReader>
#include <QMap>
#include <QSharedPointer>

class Cyclogram;
class Command;
class Version;

class FileReader
{
public:
    FileReader(QSharedPointer<Cyclogram> cyclogram);

    bool read(QIODevice* device);
    QString errorString() const;

    static Version fileVersion(const QString& fileName, bool* ok = Q_NULLPTR);
    static QString fileHash(const QString& fileName);

private:
    void readCyclogram(const Version& fileVersion);
    void readSettings(const Version& fileVersion);
    void readVariables(const Version& fileVersion);
    void readCommands(const Version& fileVersion);
    void readCommandsLinks(const Version& fileVersion);

    QWeakPointer<Cyclogram> mCyclogram;
    QMap<qint64, Command*> mCommands;
    QXmlStreamReader mXML;
};

#endif // FILE_READER_H
