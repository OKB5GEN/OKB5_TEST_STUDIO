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

void ModuleDriveSimulator::processCustomCommand()
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(mCurrentTransaction.commandID);

    switch (command)
    {
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        {
            int sensorNumber = mCurrentTransaction.inputParams.value(SystemState::SENSOR_NUMBER).toInt();
            addModuleCmd(command, sensorNumber, 0);
        }
        break;

    case ModuleCommands::RESET_LINE_1:
    case ModuleCommands::START_MEASUREMENT_LINE_1:
    case ModuleCommands::GET_DS1820_COUNT_LINE_1:
        {
            addModuleCmd(command, 0, 0);
        }
        break;

    default:
        {
            mCurrentTransaction.error = QString("Unexpected command %1 received by DS module").arg(command);
            emit commandResult(mCurrentTransaction);
            return;
        }
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
            addResponseParam(SystemState::SENSORS_COUNT, mSensorsCnt);
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        {
            int sensorNumber = mCurrentTransaction.inputParams.value(SystemState::SENSOR_NUMBER).toInt();
            double temperature = ModuleOKB::getDS1820Temp(response);
            LOG_INFO(QString("Sensor #%1 temperature is %2").arg(sensorNumber).arg(temperature));
            addResponseParam(SystemState::TEMPERATURE, temperature);
        }
        break;

    case ModuleCommands::START_MEASUREMENT_LINE_1:
    case ModuleCommands::RESET_LINE_1:
        {
            // do nothing
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
    response = mCurrentTransaction;
}

void ModuleDriveSimulator::onApplicationFinish()
{
    closePort();
}
