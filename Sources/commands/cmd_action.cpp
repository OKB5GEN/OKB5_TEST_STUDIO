#include "Headers/commands/cmd_action.h"

CmdAction::CmdAction(DRAKON::IconType type, QObject* parent):
    Command(type, parent)
{

}

void CmdAction::run()
{
    finish();
}

void CmdAction::stop()
{
}

void CmdAction::pause()
{
}

void CmdAction::resume()
{
}

void CmdAction::finish()
{
    stop();
    if (mNextCommands.empty())
    {
        emit finished(Q_NULLPTR);
        return;
    }

    emit finished(mNextCommands[0]);
}

void CmdAction::setVariableController(VariableController* controller)
{
    mVarCtrl = controller;
}

VariableController* CmdAction::variableController() const
{
    return mVarCtrl;
}
