#include "Headers/system/abstract_module.h"
#include "Headers/logger/Logger.h"

#include <QMetaEnum>

/* TODO: Эмулятор (общая задумка)
 *
 * Главная цель эмулятора - проверить ПРОГРАММУ, что ее данным можно доверять
 *
 * 1. В АбстрактномМодуле заводим флажок "эмулятор", чтобы на его основе реализовывать логику.
 * 2. Флажок этот грузится из system_config.xml
 * 3. В каждую модульную команду добавляем целочисленный параметр "ИдентификаторОтветаЭмулятора"
 * 4. В ГУИ этот параметр виден только при включенном эмуляторе
 * 5. При включенном же эмуляторе этот параметр и используется.
 * 6. Этот идентификатор нужен для того, чтобы идентифицировать ответ, который мы хотим получить
 * 7. Каким-то макаром надо добавить в конфиг эти варианты ответа с идентификаторами
 *
*/

AbstractModule::AbstractModule(QObject* parent):
    QObject(parent),
    mState(NOT_INITIALIZED),
    mIsEmulator(false)
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

void AbstractModule::setEmulator(bool isEmulator)
{
    mIsEmulator = isEmulator;
}

bool AbstractModule::isEmulator() const
{
    return mIsEmulator;
}
