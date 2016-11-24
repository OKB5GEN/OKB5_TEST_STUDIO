#include "Headers/commands/cmd_state_start.h"

CmdStateStart::CmdStateStart(const QString& name, QObject * parent):
    Command(ShapeTypes::BRANCH_BEGIN, parent)
{
    mText = name;
}
