#ifndef FILE_WRITER_H
#define FILE_WRITER_H

#include <QXmlStreamWriter>

class Cyclogram;

class FileWriter
{
public:
    FileWriter(Cyclogram* cyclogram);
    bool writeFile(QIODevice *device);

private:
    void writeVariables();
    void writeCommands();
    void writeCommandTree();

    QXmlStreamWriter mXML;
    Cyclogram* mCyclogram;
};

#endif // FILE_WRITER_H
