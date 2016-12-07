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

const QMap<QString, qreal>& VariableController::variables() const
{
    return mVariables;
}

qreal VariableController::variable(const QString& name, const qreal& defaultValue/* = -1*/) const
{
    return mVariables.value(name, defaultValue);
}

void VariableController::setVariable(const QString& name, qreal value)
{
    if (mVariables.contains(name))
    {
        mVariables[name] = value;
    }
}

void VariableController::addVariable(const QString& name, qreal value)
{
    mVariables[name] = value;
}

void VariableController::removeVariable(const QString& name)
{
    mVariables.remove(name);
}

bool VariableController::isVariableExist(const QString& name) const
{
    return mVariables.contains(name);
}

void VariableController::restart()
{

}
