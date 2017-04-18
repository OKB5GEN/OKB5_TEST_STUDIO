#ifndef FILE_WRITER_H
#define FILE_WRITER_H

#include <QXmlStreamWriter>
#include <QSharedPointer>

class Cyclogram;

class FileWriter
{
public:
    FileWriter(QSharedPointer<Cyclogram> cyclogram);
    bool writeFile(QIODevice *device);

private:
    void writeVariables();
    void writeCommands();
    void writeCommandTree();

    QXmlStreamWriter mXML;
    QWeakPointer<Cyclogram> mCyclogram;
};

#endif // FILE_WRITER_H
