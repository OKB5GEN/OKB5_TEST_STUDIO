#include <QTime>
#include <QTimer>
#include <QMetaEnum>

#include "Headers/cyclogram.h"
#include "Headers/commands/cmd_state_start.h"
#include "Headers/commands/cmd_set_state.h"
#include "Headers/commands/cmd_title.h"
#include "Headers/commands/cmd_delay.h"

//#define USE_CUSTOM_SIHLOUETTE

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
    QObject(parent)
{
}

void Cyclogram::createDefault()
{
    clear();

#ifdef USE_CUSTOM_SIHLOUETTE
    CmdTitle* begin = new CmdTitle(CmdTitle::BEGIN, this);
    CmdStateStart* branch1 = new CmdStateStart("BRANCH1", this);
    CmdDelay* delay1 = new CmdDelay(this);
    CmdSetState* toBranch2 = new CmdSetState("BRANCH2", this);
    CmdStateStart* branch2 = new CmdStateStart("BRANCH2", this);
    CmdDelay* delay2 = new CmdDelay(this);
    CmdSetState* toBranch3 = new CmdSetState("BRANCH3", this);
    CmdStateStart* branch3 = new CmdStateStart("BRANCH3", this);
    CmdTitle* end = new CmdTitle(CmdTitle::END, this);

    createPair(begin, branch1);
    createPair(branch1, delay1);
    createPair(delay1, toBranch2);
    createPair(toBranch2, branch2);
    createPair(branch2, delay2);
    createPair(delay2, toBranch3);
    createPair(toBranch3, branch3);
    createPair(branch3, end);

    mCommands.push_back(begin);
    mCommands.push_back(branch1);
    mCommands.push_back(delay1);
    mCommands.push_back(toBranch2);
    mCommands.push_back(branch2);
    mCommands.push_back(delay2);
    mCommands.push_back(toBranch3);
    mCommands.push_back(branch3);
    mCommands.push_back(end);

#else
    // empty silhouette
    CmdTitle* begin = new CmdTitle(CmdTitle::BEGIN, this);
    CmdStateStart* branch1 = new CmdStateStart("Start", this);
    CmdSetState* toBranch2 = new CmdSetState("End", this);
    CmdStateStart* branch2 = new CmdStateStart("End", this);
    CmdTitle* end = new CmdTitle(CmdTitle::END, this);

    mCommands.push_back(begin);
    mCommands.push_back(branch1);
    mCommands.push_back(toBranch2);
    mCommands.push_back(branch2);
    mCommands.push_back(end);

    createPair(begin, branch1);
    createPair(branch1, toBranch2);
    createPair(toBranch2, branch2);
    createPair(branch2, end);
#endif

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
        mState = RUNNING;
        mCurrent = mFirst;

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

    mState = STOPPED;
    mCurrent = mFirst;
}

void Cyclogram::pause()
{
    if (mState == RUNNING)
    {
        mCurrent->pause();
        mState = PAUSED;
    }
}

void Cyclogram::resume()
{
    if (mState == PAUSED)
    {
        mCurrent->resume();
        mState = RUNNING;
    }
}

Command* Cyclogram::first() const
{
    return mFirst;
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
    int TODO; // use QObject parent-child system for command tree hierarchy storage i ne ebi mosk s velosipedami;)
    for (int i = 0, sz = cmd->nextCommands().size(); i < sz; ++i)
    {
        deleteCommandTree(cmd->nextCommands()[i]);
    }

    cmd->deleteLater();
}

void Cyclogram::deleteCommand(Command* cmd)
{
    int TODO; // connect parent and child commands to each other
}

void Cyclogram::createPair(Command* parent, Command* child)
{
    parent->addCommand(child);
}

Command* Cyclogram::createCommand(DRAKON::IconType type)
{
    int TODO; // pass command parameters
    Command* cmd = Q_NULLPTR;

    switch (type)
    {
    case DRAKON::TERMINATOR:
        {
            cmd = new CmdTitle(CmdTitle::BEGIN, this);
        }
        break;
    case DRAKON::BRANCH_BEGIN:
        {
            cmd = new CmdStateStart("NEW_BRANCH", this);
        }
        break;
    case DRAKON::GO_TO_BRANCH:
        {
            cmd = new CmdSetState("NEW_BRANCH", this);
        }
        break;
    case DRAKON::DELAY:
        {
            CmdDelay* newCmd = new CmdDelay(this);
            cmd = newCmd;
        }
        break;
        //TODO not implemented
    case DRAKON::ACTION:{} break;
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
