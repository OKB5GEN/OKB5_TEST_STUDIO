#include "Headers/commands/cmd_set_state.h"

CmdSetState::CmdSetState(const QString& name, QObject * parent):
    Command(ShapeTypes::ADDRESS, parent),
    mName(name)
{
}

const QString& CmdSetState::name() const
{
    return mName;
}

void CmdSetState::setName(const QString& name)
{
    mName = name;
}
