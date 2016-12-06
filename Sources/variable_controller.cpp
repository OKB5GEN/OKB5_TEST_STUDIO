#include "Headers/variable_controller.h"

VariableController::VariableController(QObject* parent):
    QObject(parent)
{

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
