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

void AbstractModule::addResponseParam(uint32_t paramID, QVariant value)
{
    QString var = mCurrentTransaction.outputParams.value(paramID).toString();

    if (var.isEmpty())
    {
        LOG_ERROR(QString("No output variable found for paramID=%1").arg(paramID));
        return;
    }

    QList<QVariant> list;
    list.append(QVariant(var));
    list.append(value);
    mCurrentTransaction.outputParams[paramID] = list;
}
