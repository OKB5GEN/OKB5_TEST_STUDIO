#include "Headers/commands/cmd_action.h"
#include "Headers/variable_controller.h"

#include <QTimer>

CmdAction::CmdAction(DRAKON::IconType type, QObject* parent):
    Command(type, parent)
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

void CmdAction::setVariableController(VariableController* controller)
{
    mVarCtrl = controller;

    Qt::ConnectionType connection = Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection);
    connect(mVarCtrl, SIGNAL(nameChanged(const QString&,const QString&)), this, SLOT(onNameChanged(const QString&, const QString&)), connection);
    connect(mVarCtrl, SIGNAL(variableRemoved(const QString&)), this, SLOT(onVariableRemoved(const QString&)), connection);
}

VariableController* CmdAction::variableController() const
{
    return mVarCtrl;
}

void CmdAction::onNameChanged(const QString& newName, const QString& oldName)
{

}

void CmdAction::onVariableRemoved(const QString& name)
{

}
