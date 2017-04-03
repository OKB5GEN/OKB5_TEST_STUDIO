#ifndef VARIABLE_CONTROLLER_H
#define VARIABLE_CONTROLLER_H

#include <QObject>
#include <QMap>
#include <QVector>

class VariableController: public QObject
{
    Q_OBJECT

public:
    struct VariableData
    {
        VariableData():
            currentValue(0),
            initialValue(0)
        {
        }

        qreal currentValue;
        qreal initialValue;
        QString description;
        QString legend; //TODO
    };

    struct DataSnapshot
    {
        qint64 timestamp;
        QMap<QString, qreal> variables;
        QString label;
        QString subprogramFlag;
    };

    VariableController(QObject* parent);
    virtual ~VariableController();

    const QMap<QString, VariableData>& variablesData() const;
    VariableData variableData(const QString& name) const;
    void setCurrentValue(const QString& name, qreal value);
    void setInitialValue(const QString& name, qreal value);
    void setDescription(const QString& name, const QString& description);

    QString description(const QString& name) const;
    qreal currentValue(const QString& name, qreal defaultValue = -1) const;
    qreal initialValue(const QString& name, qreal defaultValue = -1) const;

    // add/remove actions are performed for all variable containers
    void addVariable(const QString& name, qreal value);
    void removeVariable(const QString& name);
    void renameVariable(const QString& newName, const QString& oldName);
    bool isVariableExist(const QString& name) const;

    // Data handling
    void makeDataSnapshot(const QString& label = "");
    void clearDataTimeline();
    const QVector<DataSnapshot>& dataTimeline() const;

    void saveReport(const QString& fileName);

    void addDataTimeline(const QVector<DataSnapshot>& dataTimeline);
    void startSubprogram(const QString& name, const QMap<QString, qreal>& variables);
    void endSubprogram(const QString& name, const QMap<QString, qreal>& variables);

public slots:
    void restart();
    void clear();

signals:
    void variableAdded(const QString& name, qreal value);
    void variableRemoved(const QString& name);
    void initialValueChanged(const QString& name, qreal value);
    void currentValueChanged(const QString& name, qreal value);
    void nameChanged(const QString& newName, const QString& oldName);
    void descriptionChanged(const QString& name, const QString& description);
    void dataSnapshotAdded(const VariableController::DataSnapshot& data);

private:
    QMap<QString, VariableData> mData;
    QVector<DataSnapshot> mDataTimeline;
};
#endif // VARIABLE_CONTROLLER_H
