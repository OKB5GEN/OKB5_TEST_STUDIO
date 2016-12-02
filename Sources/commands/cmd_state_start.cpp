#include "Headers/commands/cmd_state_start.h"

CmdStateStart::CmdStateStart(QObject * parent):
    Command(DRAKON::BRANCH_BEGIN, parent)
{
     mFlags = (Command::Selectable | Command::Editable | Command::Deletable);
}

void CmdStateStart::setText(const QString& text)
{
    mText = text;
    emit textChanged(mText);
}
