#include <QTime>
#include <QTimer>
#include <QMetaEnum>

#include "Headers/cyclogram.h"
#include "Headers/commands/cmd_state_start.h"
#include "Headers/commands/cmd_set_state.h"
#include "Headers/commands/cmd_title.h"
#include "Headers/commands/cmd_delay.h"
#include "Headers/commands/cmd_action.h"

namespace
{
    static const int COMMAND_RUN_INTERVAL = 10;

    void LogCmd(Command* cmd, const QString& state)
    {
        static QMetaEnum metaEnum;
        if (metaEnum.keyCount() == 0)
        {
            metaEnum = QMetaEnum::fromType<DRAKON::IconType>();
        }

        QString text(metaEnum.valueToKey(cmd->type()));
        text += ":";
        text += cmd->text();

        qDebug("[%s] Command %s %s", qUtf8Printable(QTime::currentTime().toString()), qUtf8Printable(text), qUtf8Printable(state));
    }
}

Cyclogram::Cyclogram(QObject * parent):
    QObject(parent),
    mState(STOPPED)
{
}

void Cyclogram::createDefault()
{
    clear();

    // empty silhouette
    Command* begin = createCommand(DRAKON::TERMINATOR);
    {
        CmdTitle* tmp = qobject_cast<CmdTitle*>(begin);
        tmp->setTitleType(CmdTitle::BEGIN);
    }

    Command* branch1 = createCommand(DRAKON::BRANCH_BEGIN);
    {
        CmdStateStart* tmp = qobject_cast<CmdStateStart*>(branch1);
        tmp->setText(tr("Start"));
    }

    Command* toBranch2 = createCommand(DRAKON::GO_TO_BRANCH);
    {

    }

    Command* branch2 = createCommand(DRAKON::BRANCH_BEGIN);
    {
        CmdStateStart* tmp = qobject_cast<CmdStateStart*>(branch2);
        tmp->setText(tr("End"));
    }

    Command* end = createCommand(DRAKON::TERMINATOR);
    {
        CmdTitle* tmp = qobject_cast<CmdTitle*>(end);
        tmp->setTitleType(CmdTitle::END);
    }

    begin->addCommand(branch1);
    branch1->addCommand(toBranch2);
    toBranch2->addCommand(branch2);
    branch2->addCommand(end);

    mFirst = begin;
    mLast = end;

    mCurrent = Q_NULLPTR;

    emit changed();
}

void Cyclogram::run()
{
    qDebug("==================================");
    qDebug("[%s] Cyclogram started", qUtf8Printable(QTime::currentTime().toString()));
    qDebug("==================================");

    if (mState == STOPPED && mFirst != Q_NULLPTR)
    {
        mCurrent = mFirst;
        setState(RUNNING);
        runCurrentCommand();
    }
}

void Cyclogram::onCommandFinished(Command* cmd)
{
    disconnect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
    LogCmd(mCurrent, "finished");

    if (cmd != Q_NULLPTR)
    {
        mCurrent = cmd;

        if (mState == RUNNING)
        {
            QTimer::singleShot(COMMAND_RUN_INTERVAL, this, SLOT(runCurrentCommand())); // to avoid direct slot after signal calling run next command after short timeout
        }
    }
    else
    {
        qDebug("==================================");
        qDebug("[%s] Cyclogram finished", qUtf8Printable(QTime::currentTime().toString()));
        qDebug("==================================");
        stop();
        emit finished();
    }
}

void Cyclogram::runCurrentCommand()
{
    if (mCurrent)
    {
        LogCmd(mCurrent, "started");
        connect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
        mCurrent->run();
    }
}

void Cyclogram::stop()
{
    if (mState == RUNNING || mState == PAUSED)
    {
        disconnect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
        mCurrent->stop();
    }

    mCurrent = mFirst;
    setState(STOPPED);
}

void Cyclogram::pause()
{
    if (mState == RUNNING)
    {
        mCurrent->pause();
        setState(PAUSED);
    }
}

void Cyclogram::resume()
{
    if (mState == PAUSED)
    {
        mCurrent->resume();
        setState(RUNNING);
    }
}

Command* Cyclogram::first() const
{
    return mFirst;
}

Command* Cyclogram::last() const
{
    return mLast;
}

Command* Cyclogram::current() const
{
    return mCurrent;
}

Cyclogram::State Cyclogram::state() const
{
    return mState;
}

void Cyclogram::clear()
{
    if (mFirst)
    {
        deleteCommandTree(mFirst);
    }

    mFirst = Q_NULLPTR;
    mCurrent = Q_NULLPTR;
    mLast = Q_NULLPTR;
    mCommands.clear();
}

void Cyclogram::deleteCommandTree(Command* cmd)
{
    emit deleted(cmd);

    for (int i = 0, sz = mCommands.size(); i < sz; ++i)
    {
        if (mCommands[i] == cmd)
        {
            mCommands.takeAt(i);
            break;
        }
    }

    for (int i = 0, sz = cmd->nextCommands().size(); i < sz; ++i)
    {
        deleteCommandTree(cmd->nextCommands()[i]);
    }

    cmd->deleteLater();
}

void Cyclogram::deleteCommand(Command* cmd, bool recursive /*= false*/)
{
    if (recursive)
    {
        deleteCommandTree(cmd);
        return;
    }

    emit deleted(cmd);

    int TODO; // this is valid for one-column branches only!
    Command* parentCmd = cmd->parentCommand();
    Command* nextCmd = cmd->nextCommands()[0]; // TODO QUESTION deletion
    parentCmd->replaceCommand(nextCmd, nextCmd->role());

    for (int i = 0, sz = mCommands.size(); i < sz; ++i)
    {
        if (mCommands[i] == cmd)
        {
            Command* tmp = mCommands.takeAt(i);
            tmp->deleteLater();
            break;
        }
    }
}

Command* Cyclogram::createCommand(DRAKON::IconType type)
{
    Command* cmd = Q_NULLPTR;

    switch (type)
    {
    case DRAKON::TERMINATOR:
        {
            cmd = new CmdTitle(this);
        }
        break;
    case DRAKON::BRANCH_BEGIN:
        {
            cmd = new CmdStateStart(this);
        }
        break;
    case DRAKON::GO_TO_BRANCH:
        {
            cmd = new CmdSetState(this);
        }
        break;
    case DRAKON::DELAY:
        {
            cmd = new CmdDelay(this);
        }
        break;
    case DRAKON::ACTION:
        {
            cmd = new CmdAction(this);
        }
        break;

        //TODO not implemented
    case DRAKON::QUESTION:{} break;
    case DRAKON::SWITCH:{} break;
    case DRAKON::CASE:{} break;
    case DRAKON::SUBPROGRAM:{} break;
    case DRAKON::SHELF:{} break;
    case DRAKON::PARAMS:{} break;
    case DRAKON::FOR_BEGIN:{} break;
    case DRAKON::FOR_END:{} break;
    case DRAKON::OUTPUT:{} break;
    case DRAKON::INPUT:{} break;
    case DRAKON::START_TIMER:{} break;
    case DRAKON::SYNCHRONIZER:{} break;
    case DRAKON::PARALLEL_PROCESS:{} break;
    default:
        break;
    }

    if (cmd)
    {
        mCommands.push_back(cmd);
    }

    return cmd;
}

const QList<Command*>& Cyclogram::commands() const
{
    return mCommands;
}

void Cyclogram::setState(State state)
{
    mState = state;
    emit stateChanged(mState);
}

Command* Cyclogram::validate() const
{
    foreach (Command* command, mCommands)
    {
        if (command->hasError())
        {
            return command;
        }
    }

    return Q_NULLPTR;
}
