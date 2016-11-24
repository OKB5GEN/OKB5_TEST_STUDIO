#include "Headers/commands/cmd_set_state.h"

CmdSetState::CmdSetState(const QString& name, QObject * parent):
    Command(ShapeTypes::GO_TO_BRANCH, parent)
{
    mText = name;
}
