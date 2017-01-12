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
    virtual ~VariableController();

    // set/get actions are usually performed on current variable values
    const QMap<QString, qreal>& variables(Container container = Current) const;
    qreal variable(const QString& name, qreal defaultValue = -1, Container container = Current) const;
    void setVariable(const QString& name, qreal value, Container container = Current);

    // add/remove actions are performed for all variable containers
    void addVariable(const QString& name, qreal value);
    void removeVariable(const QString& name);
    void renameVariable(const QString& newName, const QString& oldName);
    bool isVariableExist(const QString& name) const;

public slots:
    void restart();

signals:
    void variableAdded(const QString& name, qreal value);
    void variableRemoved(const QString& name);
    void valueChanged(const QString& name, qreal value, int container);
    void nameChanged(const QString& newName, const QString& oldName);

private:
    QMap<QString, qreal> mCurrent; //TODO create structure Variable and remove copypaste
    QMap<QString, qreal> mInitial;
};
#endif // VARIABLE_CONTROLLER_H
