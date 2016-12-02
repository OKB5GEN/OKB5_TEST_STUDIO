#include "Headers/commands/cmd_state_start.h"

CmdStateStart::CmdStateStart(QObject * parent):
    Command(DRAKON::BRANCH_BEGIN, parent)
{
}

void CmdStateStart::setText(const QString& text)
{
    mText = text;
    emit textChanged(mText);
}
