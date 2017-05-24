#include "Headers/system/modules/module_drive_simulator.h"
#include "Headers/logger/Logger.h"
#include "Headers/system/system_state.h"

namespace
{
    static const uint8_t DS_DEFAULT_ADDR = 0x11;
    static const int SERIAL_NUMBER_BYTES_COUNT = 8;
}

ModuleDriveSimulator::ModuleDriveSimulator(QObject* parent):
    ModuleOKB(parent),
    mSensorsCnt(0)
{
}

ModuleDriveSimulator::~ModuleDriveSimulator()
{

}

int ModuleDriveSimulator::sensorsCount() const
{
    return mSensorsCnt;
}

void ModuleDriveSimulator::processCustomCommand()
{
    mTemperatureData.clear();

    ModuleCommands::CommandID command = ModuleCommands::CommandID(mCurrentTransaction.commandID);

    switch (command)
    {
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        {
            LOG_INFO(QString("Start drives temperature measurement"));
            addModuleCmd(ModuleCommands::START_MEASUREMENT_LINE_1, 0, 0);
            for(int i = 0; i < mSensorsCnt; ++i)
            {
                addModuleCmd(command, i + 1, 0);
            }
        }
        break;

    case ModuleCommands::RESET_LINE_1:
        {
            addModuleCmd(command, 0, 0);
        }
        break;

    default:
        LOG_ERROR(QString("Unexpected command %1 received by DS module").arg(command));
        return;
        break;
    }
}

bool ModuleDriveSimulator::processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response)
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(operationID);

    switch (command)
    {
    case ModuleCommands::GET_DS1820_COUNT_LINE_1:
        {
            mSensorsCnt = response[2];
            LOG_INFO(QString("DS1820 sensors count is %1").arg(mSensorsCnt));
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        {
            double uu = ModuleOKB::getDS1820Temp(response);
            LOG_INFO(QString("Sensor #%1 temperature is %2").arg(mTemperatureData.size() + 1).arg(uu));
            mTemperatureData.push_back(uu);
        }
        break;

    case ModuleCommands::START_MEASUREMENT_LINE_1:
    case ModuleCommands::RESET_LINE_1:
        {
            int TODO; // check error?
        }
        break;

    default:
        LOG_WARNING(QString("Unexpected command id=0x%1 response received by DS module (HEX): %2").arg(QString::number(command, 16)).arg(QString(response.toHex().toStdString().c_str())));
        return false;
        break;
    }

    return true;
}

void ModuleDriveSimulator::onModuleError()
{
    int TODO; //TODO here will be processing
}

void ModuleDriveSimulator::createResponse(Transaction& response)
{
    switch (mCurrentTransaction.commandID)
    {
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        {
            // fill response
            int paramsCount = mCurrentTransaction.outputParams.size();
            int valuesCount = mTemperatureData.size();

            if (paramsCount != valuesCount)
            {
                LOG_ERROR(QString("Request output params count (%1) and values count (%2) mismatch").arg(paramsCount).arg(valuesCount));
                return;
            }

            QMap<uint32_t, QVariant> outputParams;
            int i = 0;
            for (auto it = mCurrentTransaction.outputParams.begin(); it != mCurrentTransaction.outputParams.end(); ++it)
            {
                //TODO replace by addResponseParam(it.key(), mTemperatureData[i]); // TODO output params malformed packet!
                QList<QVariant> list;
                list.append(it.value());
                list.append(QVariant(mTemperatureData[i]));
                outputParams[it.key()] = list;
                ++i;
            }

            mCurrentTransaction.outputParams = outputParams;
        }
        break;

    default: // just give current transaction data
        break;
    }

    response = mCurrentTransaction;
}

void ModuleDriveSimulator::onApplicationFinish()
{
    closePort();
}
