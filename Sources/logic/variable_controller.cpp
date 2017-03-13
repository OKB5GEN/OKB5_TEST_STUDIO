#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

VariableController::VariableController(QObject* parent):
    QObject(parent)
{
}

VariableController::~VariableController()
{

}

const QMap<QString, VariableController::VariableData>& VariableController::variablesData() const
{
    return mData;
}

VariableController::VariableData VariableController::variableData(const QString& name) const
{
    auto it = mData.find(name);
    if (it != mData.end())
    {
        return it.value();
    }

    return VariableData();
}

qreal VariableController::currentValue(const QString& name, qreal defaultValue) const
{
    auto it = mData.find(name);
    if (it != mData.end())
    {
        return it.value().currentValue;
    }

    return defaultValue;
}

qreal VariableController::initialValue(const QString& name, qreal defaultValue) const
{
    auto it = mData.find(name);
    if (it != mData.end())
    {
        return it.value().initialValue;
    }

    return defaultValue;
}

void VariableController::setCurrentValue(const QString& name, qreal value)
{
    auto it = mData.find(name);
    if (it != mData.end())
    {
        LOG_INFO(QString("Variable '%1' current value changed to %2").arg(name).arg(value));
        VariableData data = it.value();
        data.currentValue = value;
        mData[name] = data;
        emit currentValueChanged(name, value);
    }
}

void VariableController::setInitialValue(const QString& name, qreal value)
{
    auto it = mData.find(name);
    if (it != mData.end())
    {
        LOG_INFO(QString("Variable '%1' initial value changed to %2").arg(name).arg(value));
        VariableData data = it.value();
        data.initialValue = value;
        mData[name] = data;
        emit initialValueChanged(name, value);
    }
}

void VariableController::addVariable(const QString& name, qreal value)
{
    VariableData data;
    data.currentValue = value;
    data.initialValue = value;
    mData[name] = data;
    emit variableAdded(name, value);
}

void VariableController::removeVariable(const QString& name)
{
    mData.remove(name);
    emit variableRemoved(name);
}

void VariableController::renameVariable(const QString& newName, const QString& oldName)
{
    if (isVariableExist(oldName) && !isVariableExist(newName))
    {
        VariableData data = variableData(oldName);
        mData[newName] = data;

        emit nameChanged(newName, oldName);
    }
}

bool VariableController::isVariableExist(const QString& name) const
{
    // it is enough to check initial values container, cause all containers are synchronized
    return mData.contains(name);
}

void VariableController::restart()
{
    for (auto it = mData.begin(); it != mData.end(); ++it)
    {
        setCurrentValue(it.key(), it.value().initialValue);
    }
}

void VariableController::clear()
{
    mData.clear();
}

void VariableController::setDescription(const QString& name, const QString& description)
{
    VariableData data;
    data.currentValue = 0;
    data.initialValue = 0;

    auto it = mData.find(name);
    if (it != mData.end())
    {
        data = it.value();
    }

    bool sendSignal = (data.description != description);

    if (sendSignal)
    {
        data.description = description;
        mData[name] = data;
        emit descriptionChanged(name, description);
    }
}

QString VariableController::description(const QString& name) const
{
    auto it = mData.find(name);
    if (it != mData.end())
    {
        return it.value().description;
    }

    return "";
}
