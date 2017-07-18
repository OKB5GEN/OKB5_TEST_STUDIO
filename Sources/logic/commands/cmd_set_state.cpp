#include "Headers/logic/commands/cmd_set_state.h"
#include "Headers/logger/Logger.h"

CmdSetState::CmdSetState(QObject * parent):
    Command(DRAKON::GO_TO_BRANCH, 1, parent)
{
     mFlags = (Command::Selectable | Command::Editable);
}

void CmdSetState::setText(const QString& text)
{
    mText = text;
    emit dataChanged(mText);
}

bool CmdSetState::loadFromImpl(Command* other)
{
    CmdSetState* otherSetStateCmd = qobject_cast<CmdSetState*>(other);
    if (!otherSetStateCmd)
    {
        LOG_ERROR(QString("Command type mismatch (not go to branch)"));
        return false;
    }

    setText(otherSetStateCmd->text());
    return true;
}

