#include "Headers/system/abstract_module.h"
#include "Headers/logger/Logger.h"

#include <QMetaEnum>

AbstractModule::AbstractModule(QObject* parent):
    QObject(parent),
    mState(NOT_INITIALIZED)
{
}

void AbstractModule::setModuleID(ModuleCommands::ModuleID id)
{
    mModuleID = id;
    QMetaEnum e = QMetaEnum::fromType<ModuleCommands::ModuleID>();
    mModuleName = e.valueToKey(mModuleID);
}

ModuleCommands::ModuleID AbstractModule::moduleID() const
{
    return mModuleID;
}

void AbstractModule::setModuleState(ModuleState state, const QString& error)
{
    QMetaEnum states = QMetaEnum::fromType<AbstractModule::ModuleState>();
    ModuleState oldState = mState;
    ModuleState newState = state;

    LOG_INFO(QString("%1 module state changed from %2 to %3").arg(moduleName()).arg(states.valueToKey(oldState)).arg(states.valueToKey(newState)));

    if (!error.isEmpty())
    {
        LOG_ERROR(QString("with error: %1").arg(error));
    }

    mErrorString = error;
    mState = state;
    emit stateChanged(mModuleID, oldState, newState);
}

const QString& AbstractModule::moduleName() const
{
    return mModuleName;
}

AbstractModule::ModuleState AbstractModule::moduleState() const
{
    return mState;
}

const QString& AbstractModule::errorString() const
{
    return mErrorString;
}

