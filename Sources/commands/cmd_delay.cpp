#include "Headers/commands/cmd_delay.h"
#include <QTimer>

CmdDelay::CmdDelay(QObject* parent):
    Command(ShapeTypes::DELAY, parent),
    mDelay(0)
{
    mTimer = new QTimer(this);
    //mTimer->setSingleShot(true);

    connect(mTimer, SIGNAL(timeout()), this, SLOT(finish()));
}

void CmdDelay::run()
{
    if (!mTimer->isActive() && mDelay > 0)
    {
        mTimer->start(mDelay);
    }
}

void CmdDelay::stop()
{
    mDelay = mTimer->remainingTime();
    mTimer->stop();
}

void CmdDelay::pause()
{
    stop();
}

void CmdDelay::resume()
{
    run();
}

void CmdDelay::finish()
{
    emit onFinished(mNext);
    mTimer->stop(); // to not restart timer again
}

void CmdDelay::setDelay(int seconds)
{
     mDelay = seconds * 1000;
}
