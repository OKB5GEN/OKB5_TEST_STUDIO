#include "Headers/logic/commands/cmd_parallel_process.h"
#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

CmdParallelProcess::CmdParallelProcess(QObject* parent):
    Command(DRAKON::PARALLEL_PROCESS, 1, parent)
{
    mText = "Parallel Process";
}

void CmdParallelProcess::run()
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

void CmdParallelProcess::stop()
{
}

#ifdef ENABLE_CYCLOGRAM_PAUSE
void CmdParallelProcess::pause()
{
}

void CmdParallelProcess::resume()
{
}
#endif

void CmdParallelProcess::finish()
{
    emit finished(nextCommand());
}

void CmdParallelProcess::writeCustomAttributes(QXmlStreamWriter* writer)
{
//    writer->writeAttribute("delay", QString::number(mDelay));
}

void CmdParallelProcess::readCustomAttributes(QXmlStreamReader* reader)
{
//    QXmlStreamAttributes attributes = reader->attributes();
//    int delay = 0;
//    if (attributes.hasAttribute("delay"))
//    {
//        delay = attributes.value("delay").toInt();
//    }

//    setDelay(delay);
}
