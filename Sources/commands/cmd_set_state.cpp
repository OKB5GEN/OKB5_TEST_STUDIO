#include "Headers/commands/cmd_set_state.h"

CmdSetState::CmdSetState(QString name, QObject * parent):
    Command(parent),
    mName(name)
{
}
