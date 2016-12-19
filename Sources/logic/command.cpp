#include <QTime>
#include <QTimer>

#include "Headers/logic/command.h"
#include "Headers/logic/variable_controller.h"

namespace
{
    static const int EXECUTION_DELAY = 50;
}

Command::Command(DRAKON::IconType type, int childCmdCnt, QObject * parent):
    QObject(parent),
    mType(type),
    mRole(0),
    mFlags(Command::All),
    mParentCommand(Q_NULLPTR),
    mHasError(false),
    mExecutionDelay(0)
{
    setExecutionDelay(EXECUTION_DELAY);

    for (int i = 0; i < childCmdCnt; ++i)
    {
        mChildCommands.push_back(Q_NULLPTR);
    }
}

Command::~Command()
{

}

void Command::run()
{
    if (mExecutionDelay > 0)
    {
        QTimer::singleShot(mExecutionDelay, this, SLOT(end()));
    }
    else
    {
        end();
    }
}

void Command::end()
{
    if (mNextCommands.empty())
    {
        emit finished(Q_NULLPTR);
        return;
    }

    emit finished(mNextCommands[0]);
}

void Command::stop()
{

}

void Command::pause()
{

}

void Command::resume()
{

}

DRAKON::IconType Command::type() const
{
    return mType;
}

const QString& Command::text() const
{
    return mText;
}

const QString& Command::errorDesc() const
{
    return mErrorText;
}

void Command::setFlags(uint32_t flags)
{
    mFlags = flags;
}

uint32_t Command::flags() const
{
    return mFlags;
}

const QList<Command*>& Command::nextCommands() const
{
    return mNextCommands;
}

const QList<Command*>& Command::childCommands() const
{
    return mChildCommands;
}

void Command::addCommand(Command* cmd, int role /*= 0*/)
{
    if (cmd)
    {
        if (cmd->type() != DRAKON::BRANCH_BEGIN)
        {
            cmd->setParentCommand(this);
        }

        cmd->setRole(role);
        mNextCommands.push_back(cmd);

        if (mType == DRAKON::GO_TO_BRANCH && cmd->type() == DRAKON::BRANCH_BEGIN)
        {
            connect(cmd, SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
            onNextCmdTextChanged(cmd->text());
        }

        if (mType != DRAKON::GO_TO_BRANCH && mType != DRAKON::TERMINATOR)
        {
            setChildCommand(cmd, role);
        }
    }
}

void Command::onNextCmdTextChanged(const QString& text)
{
    mText = text;
    emit textChanged(mText);
}

int Command::role() const
{
    return mRole;
}

void Command::setRole(int role)
{
    mRole = role;
}

void Command::setActive(bool active)
{
    emit activeStateChanged(active);
}

void Command::setParentCommand(Command* cmd)
{
    mParentCommand = cmd;
}

Command* Command::parentCommand() const
{
    int TODO; // неясно которую из команд считать парентовой в случае если сверху находится развилка QUESTION или SWICH-CASE, вероятно будет тоже массив, как и "nextCommands"
    return mParentCommand;
}

bool Command::hasError() const
{
    return mHasError;
}

void Command::insertCommand(Command* newCmd, int role) // new command inserted to valency point
{
    int TODO; // непонятно как эти роли должны передаваться при вставке
    // роль - это актуально только для ветвлений
    for (int i = 0, sz = mNextCommands.size(); i < sz; ++i)
    {
        if (mNextCommands[i]->role() == role)
        {
            setChildCommand(newCmd, role);
            newCmd->setParentCommand(this);
            newCmd->setRole(role);
            newCmd->addCommand(mNextCommands[i], 0);
            mNextCommands[i] = newCmd;
            break;
        }
    }
}

void Command::setChildCommand(Command* cmd, int role)
{
    if (role >= 0 && role < mChildCommands.size())
    {
        mChildCommands[role] = cmd;
    }
}

void Command::replaceChildCommand(Command* newCmd, Command* oldCmd)
{
    for (int i = 0, sz = mChildCommands.size(); i < sz; ++i)
    {
        if (mChildCommands[i] == oldCmd)
        {
            mChildCommands[i] = newCmd;
            return;
        }
    }
}

void Command::replaceCommand(Command *newCmd, int role)
{
    int TODO2; // possibly make virtual and move to DRAKON::GO_TO_BRANCH class

    if (mType == DRAKON::GO_TO_BRANCH)
    {
        if (!newCmd)  // branch deletion
        {
            mNextCommands.clear();
            setErrorStatus(true);
            return;
        }

        if (mNextCommands.empty() && hasError()) // error fixing after branch deletion
        {
            newCmd->setRole(role); //TODO possibly not?
            mNextCommands.push_back(newCmd);
            connect(newCmd, SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
            setErrorStatus(false);
            return;
        }
    }

    int TODO; // непонятно как эти роли должны передаваться при замене
    // роль - это актуально только для ветвлений
    for (int i = 0, sz = mNextCommands.size(); i < sz; ++i)
    {
        if (mNextCommands[i]->role() == role)
        {
            if (mType == DRAKON::GO_TO_BRANCH && newCmd->type() == DRAKON::BRANCH_BEGIN)
            {
                disconnect(mNextCommands[i], SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
                connect(newCmd, SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
            }
            else
            {
                newCmd->setParentCommand(this);
            }

            mNextCommands[i] = newCmd;
            break;
        }
    }
}

void Command::setErrorStatus(bool status)
{
    mHasError = status;
    emit errorStatusChanged(mHasError);
}

void Command::setExecutionDelay(int msec)
{
    mExecutionDelay = msec;
}

void Command::setVariableController(VariableController* controller)
{
    mVarCtrl = controller;

    Qt::ConnectionType connection = Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection);
    connect(mVarCtrl, SIGNAL(nameChanged(const QString&,const QString&)), this, SLOT(onNameChanged(const QString&, const QString&)), connection);
    connect(mVarCtrl, SIGNAL(variableRemoved(const QString&)), this, SLOT(onVariableRemoved(const QString&)), connection);
}

VariableController* Command::variableController() const
{
    return mVarCtrl;
}

void Command::onNameChanged(const QString& newName, const QString& oldName)
{

}

void Command::onVariableRemoved(const QString& name)
{

}
