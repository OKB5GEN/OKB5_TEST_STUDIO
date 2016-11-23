#include "Headers/commands/cmd_state_start.h"

CmdStateStart::CmdStateStart(QString name, QObject * parent):
    Command(ShapeTypes::HEADLINE, parent),
    mName(name)
{
}

void CmdStateStart::run()
{
    emit onFinished(mNext);
}
