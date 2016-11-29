#include <QTime>

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

        if (mType == ShapeTypes::GO_TO_BRANCH && cmd->type() == ShapeTypes::BRANCH_BEGIN)
        {
            connect(cmd, SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
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

void Command::insertCommand(Command* newCmd, int role)
{
    int TODO; // непонятно как эти роли должны передаваться при вставке
    // роль - это актуально только для ветвлений
    for (int i = 0, sz = mNextCommands.size(); i < sz; ++i)
    {
        if (mNextCommands[i]->role() == role)
        {
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
            if (mType == ShapeTypes::GO_TO_BRANCH && newCmd->type() == ShapeTypes::BRANCH_BEGIN)
            {
                disconnect(mNextCommands[i], SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
                connect(newCmd, SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
            }

            mNextCommands[i] = newCmd;
            break;
        }
    }
}
