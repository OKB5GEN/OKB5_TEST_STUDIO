#include "Headers/logic/variable_controller.h"

VariableController::VariableController(QObject* parent):
    QObject(parent)
{
}

VariableController::~VariableController()
{

}

const QMap<QString, qreal>& VariableController::variables(Container container) const
{
    if (container == Current)
    {
        return mCurrent;
    }

    return mInitial;
}

qreal VariableController::variable(const QString& name, qreal defaultValue/* = -1*/, Container container) const
{
    if (container == Current)
    {
        return mCurrent.value(name, defaultValue);
    }
    else if (container == Initial)
    {
        return mInitial.value(name, defaultValue);
    }

    return defaultValue;
}

void VariableController::setVariable(const QString& name, qreal value, Container container)
{
    QMap<QString, qreal>& cont = (container == Current) ? mCurrent : mInitial;

    if (cont.contains(name))
    {
        cont[name] = value;
        emit valueChanged(name, value, container);
    }
}

void VariableController::addVariable(const QString& name, qreal value)
{
    mInitial[name] = value;
    mCurrent[name] = value;
    emit variableAdded(name, value);
}

void VariableController::removeVariable(const QString& name)
{
    mInitial.remove(name);
    mCurrent.remove(name);
    emit variableRemoved(name);
}

void VariableController::renameVariable(const QString& newName, const QString& oldName)
{
    if (isVariableExist(oldName) && !isVariableExist(newName))
    {
        qreal initialValue = mInitial.value(oldName, -1);
        qreal currentValue = mCurrent.value(oldName, -1);

        blockSignals(true);
        removeVariable(oldName);
        addVariable(newName, initialValue);
        setVariable(newName, currentValue, Current);
        blockSignals(false);

        emit nameChanged(newName, oldName);
    }
}

bool VariableController::isVariableExist(const QString& name) const
{
    // it is enough to check initial values container, cause all containers are synchronized
    return mInitial.contains(name);
}

void VariableController::restart()
{
    foreach (QString key, mInitial.keys())
    {
        setVariable(key, variable(key, -1, Initial), Current);
    }
}
