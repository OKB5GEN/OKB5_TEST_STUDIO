#include "Headers/logic/commands/cmd_set_state.h"

CmdSetState::CmdSetState(QObject * parent):
    Command(DRAKON::GO_TO_BRANCH, 0, parent)
{
     mFlags = (Command::Selectable | Command::Editable);
}

void CmdSetState::setText(const QString& text)
{
    //mText = tr("To state '%1'").arg(text);
    mText = text;
    emit textChanged(mText);
}

