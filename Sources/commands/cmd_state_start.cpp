#include "Headers/commands/cmd_state_start.h"

CmdStateStart::CmdStateStart(const QString& name, QObject * parent):
    Command(ShapeTypes::HEADLINE, parent),
    mName(name)
{
}

void CmdStateStart::run()
{
    if (mNextCommands.empty())
    {
        emit onFinished(Q_NULLPTR);
        return;
    }

    emit onFinished(mNextCommands[0]);
}

const QString& CmdStateStart::name() const
{
    return mName;
}

void CmdStateStart::setName(const QString& name)
{
    mName = name;
}
