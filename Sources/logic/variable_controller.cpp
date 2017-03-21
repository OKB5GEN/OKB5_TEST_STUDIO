#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

#include "xlsxdocument.h"

#include <QDateTime>

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
        removeVariable(oldName);
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

    clearDataTimeline();
}

void VariableController::clear()
{
    mData.clear();
    clearDataTimeline();
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

void VariableController::makeDataSnapshot(const QString& label)
{
    bool isFirstPoint = mDataTimeline.isEmpty();

    mDataTimeline.push_back(DataSnapshot());

    mDataTimeline.back().label = label;
    mDataTimeline.back().timestamp = QDateTime::currentMSecsSinceEpoch();

    for (auto it = mData.begin(); it != mData.end(); ++it)
    {
        mDataTimeline.back().variables[it.key()] = it.value().currentValue;
    }

    emit dataSnapshotAdded(mDataTimeline.back());
    int TODO; // optimize memory usage
}

void VariableController::clearDataTimeline()
{
    mDataTimeline.clear();
}

const QVector<VariableController::DataSnapshot>& VariableController::dataTimeline() const
{
    return mDataTimeline;
}

void VariableController::createDependence(const QString& xVar, const QString& yVar, QList<qreal>& x, QList<qreal>& y) const
{
    if (!mData.contains(xVar))
    {
        LOG_ERROR(QString("Can not create dependence. Variable '%1' not found").arg(xVar));
        return;
    }

    if (!mData.contains(yVar))
    {
        LOG_ERROR(QString("Can not create dependence. Variable '%1' not found").arg(yVar));
        return;
    }

    if (mDataTimeline.isEmpty())
    {
        LOG_ERROR(QString("Can not create dependence. Data timeline is empty"));
        return;
    }

    for (int i = 0, sz = mDataTimeline.size(); i < sz; ++i)
    {
        x.append(mDataTimeline[i].variables.value(xVar, 0));
        y.append(mDataTimeline[i].variables.value(yVar, 0));
    }
}

void VariableController::timeline(const QString& var, QList<qreal>& time, QList<qreal>& value) const
{
    if (!mData.contains(var))
    {
        LOG_ERROR(QString("Can not create timeline. Variable '%1' not found").arg(var));
        return;
    }

    if (mDataTimeline.isEmpty())
    {
        LOG_ERROR(QString("Can not create timeline. Data timeline is empty"));
        return;
    }

    for (int i = 0, sz = mDataTimeline.size(); i < sz; ++i)
    {
        time.append(qreal(mDataTimeline[i].timestamp - mDataTimeline.front().timestamp));
        value.append(mDataTimeline[i].variables.value(var, 0));
    }
}

void VariableController::saveReport(const QString& fileName)
{
    QXlsx::Document xlsx;
    int row = 1;
    int column = 1;

    // create data header
    xlsx.write(row, column, "Time");
    column++;

    for (auto it = mData.begin(); it != mData.end(); ++it)
    {
        xlsx.write(row, column, it.key());
        ++column;
    }

    ++row;

    // get test start time
    qint64 startTime = 0;
    if (!mDataTimeline.isEmpty())
    {
        startTime = mDataTimeline.front().timestamp;
    }

    // write data
    for (int i = 0, sz = mDataTimeline.size(); i < sz; ++i)
    {
        column = 1;
        qint64 time = mDataTimeline[i].timestamp - startTime;
        xlsx.write(row, column, time);
        ++column;

        for (auto it = mDataTimeline[i].variables.begin(); it != mDataTimeline[i].variables.end(); ++it)
        {
            xlsx.write(row, column, it.value());
            ++column;
        }

        ++row;
    }

    xlsx.saveAs(fileName);
}
