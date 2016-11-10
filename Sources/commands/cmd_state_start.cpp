#include "Headers/commands/cmd_state_start.h"

CmdStateStart::CmdStateStart(QString name, QObject * parent):
    Command(parent),
    mName(name)
{
}
