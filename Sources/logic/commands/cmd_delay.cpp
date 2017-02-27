#include "Headers/logic/commands/cmd_delay.h"
#include <QTimer>
#include <QTime>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

CmdDelay::CmdDelay(QObject* parent):
    Command(DRAKON::DELAY, 1, parent),
    mTimeLeft(0)
{
    mTimer = new QTimer(this);
    mTimer->setSingleShot(true);

    connect(mTimer, SIGNAL(timeout()), this, SLOT(finish()));
    setDelay(0, 0, 0, 0);
}

void CmdDelay::run()
{
    if (mTimer->remainingTime() > 0)
    {
        return;
    }

    if (mDelay > 0)
    {
        if (mTimeLeft > 0) // resume execution
        {
            mTimer->start(mTimeLeft);
        }
        else
        {
            mTimer->start(mDelay);
        }
    }
    else
    {
        finish();
    }
}

void CmdDelay::stop()
{
    mTimeLeft = 0;
    mTimer->stop();
}

#ifdef ENABLE_CYCLOGRAM_PAUSE
void CmdDelay::pause()
{
    int timeLeft = mTimer->remainingTime();
    stop();
    mTimeLeft = timeLeft;
}

void CmdDelay::resume()
{
    run();
}
#endif

void CmdDelay::finish()
{
    stop();
    emit finished(nextCommand());
}

int CmdDelay::delay() const
{
    return mDelay;
}

void CmdDelay::setDelay(int hours, int minutes, int seconds, int msec)
{
     mDelay = (hours * 3600 + minutes * 60 + seconds) * 1000 + msec;
     mText = tr("Wait:");
     if (hours > 0)
     {
         mText += QString::number(hours);
         mText += tr("h");
     }

     if (minutes > 0)
     {
         mText += QString::number(minutes);
         mText += tr("m");
     }

     if (hours > 0)
     {
         emit textChanged(mText);
         return;
     }

     if (seconds > 0)
     {
         mText += QString::number(seconds);
         mText += tr("s");
     }

     if (minutes > 0)
     {
         emit textChanged(mText);
         return;
     }

     if (msec > 0)
     {
         mText += QString::number(msec);
         mText += tr("ms");
         emit textChanged(mText);
         return;
     }

     if (seconds == 0)
     {
         mText += QString::number(seconds);
         mText += tr("s");
     }

     emit textChanged(mText);
}

void CmdDelay::writeCustomAttributes(QXmlStreamWriter* writer)
{
    writer->writeAttribute("delay", QString::number(mDelay));
}

void CmdDelay::readCustomAttributes(QXmlStreamReader* reader)
{
    QXmlStreamAttributes attributes = reader->attributes();
    int delay = 0;
    if (attributes.hasAttribute("delay"))
    {
        delay = attributes.value("delay").toInt();
    }

    setDelay(delay);
}

void CmdDelay::setDelay(int msec)
{
    int delay = msec;

    int hours = delay / (3600 * 1000);
    delay -= hours * 3600 * 1000;

    int minutes = delay / (60 * 1000);
    delay -= minutes * 60 * 1000;

    int seconds = delay / 1000;
    delay -= seconds * 1000;

    int milliseconds = delay;

    setDelay(hours, minutes, seconds, milliseconds);

}
