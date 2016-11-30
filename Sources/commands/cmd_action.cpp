#include "Headers/commands/cmd_action.h"

CmdAction::CmdAction(QObject* parent):
    Command(DRAKON::ACTION, parent)
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
