#include "Headers/commands/cmd_delay.h"
#include <QTimer>
#include <QTime>

CmdDelay::CmdDelay(QObject* parent):
    Command(ShapeTypes::DELAY, parent),
    mTimeLeft(0)
{
    mTimer = new QTimer(this);

    connect(mTimer, SIGNAL(timeout()), this, SLOT(finish()));
    setDelay(0, 0, 0, 0);
}

void CmdDelay::run()
{
    if (!mTimer->isActive())
    {
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
}

void CmdDelay::stop()
{
    mTimeLeft = 0;
    mTimer->stop();
}

void CmdDelay::pause()
{
    stop();
    mTimeLeft = mTimer->remainingTime();
}

void CmdDelay::resume()
{
    run();
}

void CmdDelay::finish()
{
    stop();
    if (mNextCommands.empty())
    {
        emit onFinished(Q_NULLPTR);
        return;
    }

    emit onFinished(mNextCommands[0]);
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
         return;
     }

     if (seconds > 0)
     {
         mText += QString::number(seconds);
         mText += tr("s");
     }

     if (minutes > 0)
     {
         return;
     }

     if (msec > 0)
     {
         mText += QString::number(msec);
         mText += tr("ms");
         return;
     }

     if (seconds == 0)
     {
         mText += QString::number(seconds);
         mText += tr("s");
     }
}
