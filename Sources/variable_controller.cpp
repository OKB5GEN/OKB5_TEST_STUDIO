#include "Headers/variable_controller.h"

VariableController::VariableController(QObject* parent):
    QObject(parent)
{
    int TODO; // temporary
    addVariable("I", 0);
    addVariable("J", 1);
    addVariable("K", 2);
    addVariable("N", 3);
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
    }
}

void VariableController::addVariable(const QString& name, qreal value)
{
    mInitial[name] = value;
    mCurrent[name] = value;
}

void VariableController::removeVariable(const QString& name)
{
    mInitial.remove(name);
    mCurrent.remove(name);
}

bool VariableController::isVariableExist(const QString& name) const
{
    // it is enough to check initial values container, cause all containers are synchronized
    return mInitial.contains(name);
}

void VariableController::restart()
{
    mCurrent.clear();

    mCurrent = mInitial; // perform deep copy of the container
    mCurrent.detach();
}
