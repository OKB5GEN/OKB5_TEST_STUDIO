#include "Headers/commands/cmd_state_start.h"

CmdStateStart::CmdStateStart(const QString& name, QObject * parent):
    Command(DRAKON::BRANCH_BEGIN, parent)
{
    mText = name;
}

void CmdStateStart::setText(const QString& text)
{
    mText = text;
    emit textChanged(mText);
}
