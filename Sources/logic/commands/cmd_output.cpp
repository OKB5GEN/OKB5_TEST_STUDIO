#include "Headers/logic/commands/cmd_output.h"
#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

CmdOutput::CmdOutput(QObject* parent):
    Command(DRAKON::OUTPUT, 1, parent)
{
    mText = tr("Message");
}

void CmdOutput::run()
{
    if (mExecutionDelay > 0)
    {
        QTimer::singleShot(mExecutionDelay, this, SLOT(finish()));
    }
    else
    {
        finish();
    }
}

void CmdOutput::stop()
{
}

void CmdOutput::finish()
{
    emit finished(nextCommand());
}

void CmdOutput::writeCustomAttributes(QXmlStreamWriter* writer)
{
//    writer->writeAttribute("delay", QString::number(mDelay));
}

void CmdOutput::readCustomAttributes(QXmlStreamReader* reader)
{
//    QXmlStreamAttributes attributes = reader->attributes();
//    int delay = 0;
//    if (attributes.hasAttribute("delay"))
//    {
//        delay = attributes.value("delay").toInt();
//    }
}

bool CmdOutput::loadFromImpl(Command* other)
{
    return true;
}
