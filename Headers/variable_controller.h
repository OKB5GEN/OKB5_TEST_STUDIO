#ifndef VARIABLE_CONTROLLER_H
#define VARIABLE_CONTROLLER_H

#include <QObject>
#include <QMap>

class VariableController: public QObject
{
    Q_OBJECT

public:
    enum Container
    {
        Current,
        Initial
    };

    VariableController(QObject* parent);
    ~VariableController();

    // set/get actions are usually performed on current variable values
    const QMap<QString, qreal>& variables(Container container = Current) const;
    qreal variable(const QString& name, qreal defaultValue = -1, Container container = Current) const;
    void setVariable(const QString& name, qreal value, Container container = Current);

    // add/remove actions are performed for all variable containers
    void addVariable(const QString& name, qreal value);
    void removeVariable(const QString& name);
    bool isVariableExist(const QString& name) const;

public slots:
    void restart();

private:
    QMap<QString, qreal> mCurrent;
    QMap<QString, qreal> mInitial;
};
#endif // VARIABLE_CONTROLLER_H
