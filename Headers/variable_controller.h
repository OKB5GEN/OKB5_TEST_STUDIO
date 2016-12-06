#ifndef VARIABLE_CONTROLLER_H
#define VARIABLE_CONTROLLER_H

#include <QObject>
#include <QMap>

class VariableController: public QObject
{
    Q_OBJECT

public:
    VariableController(QObject* parent);
    ~VariableController();

    const QMap<QString, qreal>& variables() const;
    qreal variable(const QString& name, const qreal& defaultValue = -1) const;
    void setVariable(const QString& name, qreal value);
    void addVariable(const QString& name, qreal value);
    void removeVariable(const QString& name);
    bool isVariableExist(const QString& name) const;

private:
    QMap<QString, qreal> mVariables;
};
#endif // VARIABLE_CONTROLLER_H
