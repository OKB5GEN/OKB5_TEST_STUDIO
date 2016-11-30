#include "Headers/commands/cmd_set_state.h"

CmdSetState::CmdSetState(const QString& name, QObject * parent):
    Command(DRAKON::GO_TO_BRANCH, parent)
{
    mText = name;
}

void CmdSetState::setText(const QString& text)
{
    mText = text;
    emit textChanged(mText);
}

