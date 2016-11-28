#include "Headers/commands/cmd_delay.h"
#include <QTimer>
#include <QTime>

CmdDelay::CmdDelay(QObject* parent):
    Command(ShapeTypes::DELAY, parent)
{
    mTimer = new QTimer(this);
    //mTimer->setSingleShot(true);

    connect(mTimer, SIGNAL(timeout()), this, SLOT(finish()));
    setDelay(0);
}

void CmdDelay::run()
{
    qDebug("[%s] Run command %s", qUtf8Printable(QTime::currentTime().toString()), qUtf8Printable(text()));

    if (!mTimer->isActive() && mDelay > 0)
    {
        mTimer->start(mDelay);
    }
}

void CmdDelay::stop()
{
    mDelay = mTimer->remainingTime();
    mTimer->stop();
    int TODO; // теоретически возможно, что mDelay == 0, а сигнал еще не посылался или уже послался
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
    qDebug("[%s] Run command %s", qUtf8Printable(QTime::currentTime().toString()), qUtf8Printable(text()));

    mTimer->stop(); // to not restart timer again
    if (mNextCommands.empty())
    {
        emit onFinished(Q_NULLPTR);
        return;
    }

    emit onFinished(mNextCommands[0]);

}

void CmdDelay::setDelay(int seconds)
{
     mDelay = seconds * 1000;
     mText = tr("Wait:%1 s").arg(QString::number(seconds));
}
