#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

#include "xlsxdocument.h"

#include <QDateTime>

namespace
{
    static const QString START_FLAG = "START";
    static const QString END_FLAG = "END";
    static const QString DELIMITER = " ";

    //static const qreal PRECISION = 0.0001;
}

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
        VariableData data = it.value();

        // do not set value, if variable does not changed significantly
        // to prevent multiple currentValueChanged signals

        //if (qAbs(data.currentValue - value) > PRECISION) // commented what if value actually was not changed, for example temperature?
        {
            LOG_INFO(QString("Variable '%1' current value changed to %2").arg(name).arg(value));

            data.currentValue = value;
            mData[name] = data;
            emit currentValueChanged(name, value);
        }
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

    int columnShift = 0;

    // write data
    for (int i = 0, sz = mDataTimeline.size(); i < sz; ++i)
    {
        column = 1;
        qint64 time = mDataTimeline[i].timestamp - startTime;
        xlsx.write(row, column, time);
        ++column;

        if (mDataTimeline[i].subprogramFlag.isEmpty())
        {
            for (auto it = mDataTimeline[i].variables.begin(); it != mDataTimeline[i].variables.end(); ++it)
            {
                xlsx.write(row, columnShift + column, it.value());
                ++column;
            }

            if (!mDataTimeline[i].label.isEmpty())
            {
                xlsx.write(row, columnShift + column, mDataTimeline[i].label);
                ++column;
            }
        }
        else // subprogram start/end
        {
            QStringList token = mDataTimeline[i].subprogramFlag.split(DELIMITER);
            if (token.size() != 2)
            {
                LOG_ERROR(QString("Invalid subprogram name: '%1'").arg(mDataTimeline[i].subprogramFlag));
                continue;
            }

            if (token.at(0) == START_FLAG)
            {
                // shift right table start column to start write subprogram variables
                if (columnShift == 0)
                {
                    columnShift += mData.keys().size();
                }
                else
                {
                    columnShift += mDataTimeline[i - 1].variables.size();
                }

                // write subprogram name + variables header
                xlsx.write(row, columnShift + column, token.at(1));
                ++column;

                for (auto it = mDataTimeline[i].variables.begin(); it != mDataTimeline[i].variables.end(); ++it)
                {
                    xlsx.write(row, columnShift + column, it.key());
                    ++column;
                }

                ++columnShift;
            }
            else if (token.at(0) == END_FLAG)
            {
                columnShift -= mDataTimeline[i].variables.size(); // shift back table start column to write calling cyclogram variables further
                --columnShift;
                --row; // just skip currnt timeline stamp just indicating that current subprogram is finished
            }
        }

        ++row;
    }

    xlsx.saveAs(fileName);
}

void VariableController::addDataTimeline(const QVector<DataSnapshot>& dataTimeline)
{
    mDataTimeline.append(dataTimeline);
}

void VariableController::startSubprogram(const QString& name, const QMap<QString, qreal>& variables)
{
    //LOG_DEBUG(QString("VC: Start subprogram '%1'").arg(name));
    mDataTimeline.push_back(DataSnapshot());
    mDataTimeline.back().timestamp = QDateTime::currentMSecsSinceEpoch();
    mDataTimeline.back().subprogramFlag = START_FLAG + DELIMITER + name;
    mDataTimeline.back().variables = variables;
}

void VariableController::endSubprogram(const QString& name, const QMap<QString, qreal>& variables)
{
    //LOG_DEBUG(QString("VC: End subprogram '%1'").arg(name));
    mDataTimeline.push_back(DataSnapshot());
    mDataTimeline.back().timestamp = QDateTime::currentMSecsSinceEpoch();
    mDataTimeline.back().subprogramFlag = END_FLAG + DELIMITER + name;
    mDataTimeline.back().variables = variables;
}
