#include "Headers/system/abstract_module.h"
#include "Headers/logger/Logger.h"

#include <QMetaEnum>

AbstractModule::AbstractModule(QObject* parent):
    QObject(parent),
    mIsEmulator(false), // using device by default
    mIsLogicallyActive(true) // module is logically active by default
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

const QString& AbstractModule::moduleName() const
{
    return mModuleName;
}

const QString& AbstractModule::errorString() const
{
    return mErrorString;
}

void AbstractModule::setEmulator(bool isEmulator)
{
    mIsEmulator = isEmulator;
}

bool AbstractModule::isEmulator() const
{
    return mIsEmulator;
}

bool AbstractModule::isPhysicallyActive() const
{
    return true; // by default module is physically active
}

bool AbstractModule::isLogicallyActive() const
{
    return mIsLogicallyActive;
}

bool AbstractModule::isAvailable() const
{
    return mIsLogicallyActive && isPhysicallyActive();
}

void AbstractModule::setLogicallyActive(bool active)
{
    mIsLogicallyActive = active;
}
