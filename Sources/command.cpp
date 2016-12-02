#include <QTime>

#include "Headers/command.h"

Command::Command(DRAKON::IconType type, QObject * parent):
    QObject(parent),
    mType(type),
    mRole(0),
    mFlags(Command::All),
    mParentCommand(Q_NULLPTR) // TODO BRANCH_BEGIN has not parent command
{

}

Command::~Command()
{

}

void Command::run()
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

QString Command::text() const
{
    return mText;
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

void Command::setParentCommand(Command* cmd)
{
    mParentCommand = cmd;
}

Command* Command::parentCommand() const
{
    return mParentCommand;
}

void Command::insertCommand(Command* newCmd, int role) // new command inserted to valency point
{
    int TODO; // непонятно как эти роли должны передаваться при вставке
    // роль - это актуально только для ветвлений
    for (int i = 0, sz = mNextCommands.size(); i < sz; ++i)
    {
        if (mNextCommands[i]->role() == role)
        {
            newCmd->setParentCommand(this);
            newCmd->setRole(role);
            newCmd->addCommand(mNextCommands[i], 0);
            mNextCommands[i] = newCmd;
            break;
        }
    }
}

void Command::replaceCommand(Command *newCmd, int role)
{
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
