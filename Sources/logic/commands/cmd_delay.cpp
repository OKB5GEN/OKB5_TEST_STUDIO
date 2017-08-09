#include "Headers/logic/commands/cmd_delay.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

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
    setDelay(0);
}

void CmdDelay::run()
{
    if (mTimer->remainingTime() > 0)
    {
        return;
    }

    int delay = 0;
    if (!mVariable.isEmpty())
    {
        delay = mVarCtrl->currentValue(mVariable);
    }

    if (delay <= 0)
    {
        delay = mDelay;
    }

    if (delay > 0)
    {
        if (mTimeLeft > 0) // resume execution
        {
            mTimer->start(mTimeLeft);
        }
        else
        {
            mTimer->start(delay);
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
    if (hasError())
    {
        setErrorStatus(false);
    }

    mVariable.clear();

    QString text;
    mDelay = (hours * 3600 + minutes * 60 + seconds) * 1000 + msec;

    if (hours > 0)
    {
        text += QString::number(hours);
        text += tr("h");
    }

    if (minutes > 0)
    {
        text += QString::number(minutes);
        text += tr("m");
    }

    if (hours > 0)
    {
        setText(text);
        return;
    }

    if (seconds > 0)
    {
        text += QString::number(seconds);
        text += tr("s");
    }

    if (minutes > 0)
    {
        setText(text);
        return;
    }

    if (msec > 0)
    {
        text += QString::number(msec);
        text += tr("ms");
        setText(text);
        return;
    }

    if (seconds == 0)
    {
        text += QString::number(seconds);
        text += tr("s");
    }

    setText(text);
}

void CmdDelay::setText(const QString& text)
{
    QString textBefore = mText;

    if (text != tr("Invalid cmd"))
    {
        mText = tr("Wait ") + text;
    }
    else
    {
        mText = text;
    }

    if (textBefore != mText)
    {
        emit dataChanged(mText);
    }
}

void CmdDelay::writeCustomAttributes(QXmlStreamWriter* writer)
{
    writer->writeAttribute("variable", mVariable);
    writer->writeAttribute("delay", QString::number(mDelay));
}

void CmdDelay::readCustomAttributes(QXmlStreamReader* reader, const Version& fileVersion)
{
    QXmlStreamAttributes attributes = reader->attributes();
    int delay = 0;
    if (attributes.hasAttribute("delay"))
    {
        delay = attributes.value("delay").toInt();
    }

    QString variable;
    if (attributes.hasAttribute("variable"))
    {
        variable = attributes.value("variable").toString();
    }

    if (variable.isEmpty())
    {
        setDelay(delay);
    }
    else
    {
        setDelay(variable);
    }
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

bool CmdDelay::loadFromImpl(Command* other)
{
    CmdDelay* otherDelay = qobject_cast<CmdDelay*>(other);
    if (!otherDelay)
    {
        LOG_ERROR(QString("Command type mismatch (not delay)"));
        return false;
    }

    if (!otherDelay->variable().isEmpty())
    {
        setDelay(otherDelay->variable());
    }
    else
    {
        setDelay(otherDelay->delay());
    }

    return true;
}

void CmdDelay::setDelay(const QString& variable)
{
    mVariable = variable;
    mDelay = 0;

    bool isExist = mVarCtrl->isVariableExist(mVariable);

    if (hasError() && isExist)
    {
        setErrorStatus(false);
    }

    if (!hasError() && !isExist)
    {
        setErrorStatus(true);
    }

    if (hasError())
    {
        setText(tr("Invalid cmd"));
    }
    else
    {
        setText(mVariable + QString(" ") + tr("ms"));
    }
}

const QString& CmdDelay::variable() const
{
    return mVariable;
}

void CmdDelay::onNameChanged(const QString& newName, const QString& oldName)
{
    if (mVariable.isEmpty())
    {
        return;
    }

    if (mVariable == oldName)
    {
        setDelay(newName);
    }
}

void CmdDelay::onVariableRemoved(const QString& name)
{
    if (mVariable.isEmpty())
    {
        return;
    }

    if (mVariable == name)
    {
        setDelay(0);
    }
}
