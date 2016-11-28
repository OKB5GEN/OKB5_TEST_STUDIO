#include <QTime>

#include "Headers/cyclogram.h"
#include "Headers/commands/cmd_state_start.h"
#include "Headers/commands/cmd_set_state.h"
#include "Headers/commands/cmd_title.h"
#include "Headers/commands/cmd_delay.h"

//#define USE_CUSTOM_SIHLOUETTE

namespace
{
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
#else
    // empty silhouette
    CmdTitle* begin = new CmdTitle(CmdTitle::BEGIN, this);
    CmdStateStart* branch1 = new CmdStateStart("BRANCH1", this);
    CmdSetState* toBranch2 = new CmdSetState("BRANCH2", this);
    CmdStateStart* branch2 = new CmdStateStart("BRANCH2", this);
    CmdTitle* end = new CmdTitle(CmdTitle::END, this);

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
    qDebug("[%s] Cyclogram started", qUtf8Printable(QTime::currentTime().toString()));

    if (mState == STOPPED && mFirst != Q_NULLPTR)
    {
        mState = RUNNING;
        connect(mFirst, SIGNAL(onFinished(Command*)), this, SLOT(onCommandFinished(Command*)));
        mCurrent = mFirst;
        mCurrent->run();
    }
}

void Cyclogram::onCommandFinished(Command* cmd)
{
    if (cmd != Q_NULLPTR)
    {
        mCurrent = cmd;
        connect(mCurrent, SIGNAL(onFinished(Command*)), this, SLOT(onCommandFinished(Command*))); // TODO must be called on command creation

        if (mState == RUNNING)
        {
            mCurrent->run();
        }
    }
    else
    {
        stop();
    }
}

void Cyclogram::stop()
{
    qDebug("[%s] Cyclogram finished", qUtf8Printable(QTime::currentTime().toString()));

    if (mState == RUNNING || mState == PAUSED)
    {
        mCurrent->stop();
        mState = STOPPED;
        mCurrent = mFirst;
    }
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

void Cyclogram::clear()
{
    if (mFirst)
    {
        deleteCommandTree(mFirst);
    }

    mFirst = Q_NULLPTR;
    mCurrent = Q_NULLPTR;
    mLast = Q_NULLPTR;
}

void Cyclogram::deleteCommandTree(Command* cmd)
{
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

Command* Cyclogram::createCommand(ShapeTypes type)
{
    int TODO; // pass command parameters
    Command* cmd = Q_NULLPTR;

    switch (type)
    {
    case ShapeTypes::TERMINATOR:
        {
            cmd = new CmdTitle(CmdTitle::BEGIN, this);
        }
        break;
    case ShapeTypes::BRANCH_BEGIN:
        {
            cmd = new CmdStateStart("NEW_BRANCH", this);
        }
        break;
    case ShapeTypes::GO_TO_BRANCH:
        {
            cmd = new CmdSetState("NEW_BRANCH", this);
        }
        break;
    case ShapeTypes::DELAY:
        {
            CmdDelay* newCmd = new CmdDelay(this);
            int TODO2;
            newCmd->setDelay(10);
            cmd = newCmd;
        }
        break;
        //TODO not implemented
    case ShapeTypes::ACTION:{} break;
    case ShapeTypes::QUESTION:{} break;
    case ShapeTypes::CHOICE:{} break;
    case ShapeTypes::CASE:{} break;
    case ShapeTypes::INSERTION:{} break;
    case ShapeTypes::SHELF:{} break;
    case ShapeTypes::PARAMS:{} break;
    case ShapeTypes::FOR_BEGIN:{} break;
    case ShapeTypes::FOR_END:{} break;
    case ShapeTypes::OUTPUT:{} break;
    case ShapeTypes::INPUT:{} break;
    case ShapeTypes::START_TIMER:{} break;
    case ShapeTypes::SYNCHRONIZER:{} break;
    case ShapeTypes::PARALLEL_PROCESS:{} break;
    case ShapeTypes::CONCURRENT_PROCESS:{} break;
    default:
        break;
    }

    return cmd;
}
