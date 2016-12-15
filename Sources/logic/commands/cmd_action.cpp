#include "Headers/logic/commands/cmd_action.h"
#include "Headers/logic/variable_controller.h"

#include <QTimer>

CmdAction::CmdAction(DRAKON::IconType type, QObject* parent):
    Command(type, 1, parent)
{

}

void CmdAction::run()
{
    if (mExecutionDelay > 0)
    {
        QTimer::singleShot(mExecutionDelay, this, SLOT(finish()));
    }
    else
    {
        finish();
    }
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

