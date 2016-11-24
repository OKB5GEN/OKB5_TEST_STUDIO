#include "Headers/command.h"

Command::Command(ShapeTypes type, QObject * parent):
    QObject(parent),
    mType(type),
    mRole(0)
{

}

Command::~Command()
{

}

void Command::run()
{

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

ShapeTypes Command::type() const
{
    return mType;
}

QString Command::text() const
{
    return mText;
}

const QList<Command*>& Command::nextCommands() const
{
    return mNextCommands;
}

void Command::addCommand(Command* cmd, int role /*= 0*/)
{
    if (cmd)
    {
        cmd->setRole(role);
        mNextCommands.push_back(cmd);
    }
}

int Command::role() const
{
    return mRole;
}

void Command::setRole(int role)
{
    mRole = role;
}

