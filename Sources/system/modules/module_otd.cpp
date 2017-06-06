#include "Headers/system/modules/module_otd.h"
#include "Headers/logger/Logger.h"
#include "Headers/system/system_state.h"

namespace
{
    static const uint8_t OTD_DEFAULT_ADDR = 0x44;
    static const int SERIAL_NUMBER_BYTES_COUNT = 8;
    static const int MAX_PT100_COUNT = 2;
}

ModuleOTD::ModuleOTD(QObject* parent):
    ModuleOKB(parent),
    mSensorsCntPsy(0),
    mSensorsCntNu(0)
{
}

ModuleOTD::~ModuleOTD()
{

}

void ModuleOTD::processCustomCommand()
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(mCurrentTransaction.commandID);

    switch (command)
    {
    case ModuleCommands::GET_TEMPERATURE_PT100:
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
        {
            int sensorNumber = mCurrentTransaction.inputParams.value(SystemState::SENSOR_NUMBER).toInt();
            addModuleCmd(command, sensorNumber, 0);
        }
        break;

    case ModuleCommands::RESET_LINE_1:
    case ModuleCommands::RESET_LINE_2:
    case ModuleCommands::START_MEASUREMENT_LINE_1:
    case ModuleCommands::START_MEASUREMENT_LINE_2:
    case ModuleCommands::GET_DS1820_COUNT_LINE_1:
    case ModuleCommands::GET_DS1820_COUNT_LINE_2:
        {
            addModuleCmd(command, 0, 0);
        }
        break;

    default:
        {
            mCurrentTransaction.error = QString("Unexpected command %1 received by OTD module").arg(command);
            emit commandResult(mCurrentTransaction);
            return;
        }
        break;
    }
}

bool ModuleOTD::processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response)
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(operationID);

    switch (command)
    {
    case ModuleCommands::GET_DS1820_COUNT_LINE_1:
        {
            mSensorsCntPsy = response[2];

            LOG_INFO(QString("DS1820 sensors count at line 1 is %1").arg(mSensorsCntPsy));
            addResponseParam(SystemState::SENSORS_COUNT, mSensorsCntPsy);
        }
        break;

    case ModuleCommands::GET_DS1820_COUNT_LINE_2:
        {
            mSensorsCntNu = response[2];

            LOG_INFO(QString("DS1820 sensors count at line 2 is %1").arg(mSensorsCntNu));
            addResponseParam(SystemState::SENSORS_COUNT, mSensorsCntNu);
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        {
            int sensorNumber = mCurrentTransaction.inputParams.value(SystemState::SENSOR_NUMBER).toInt();
            double temperature = ModuleOKB::getDS1820Temp(response);

            LOG_INFO(QString("Sensor #%1 at line 1 temperature is %2").arg(sensorNumber).arg(temperature));
            addResponseParam(SystemState::TEMPERATURE, temperature);
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
        {
            int sensorNumber = mCurrentTransaction.inputParams.value(SystemState::SENSOR_NUMBER).toInt();
            double temperature = ModuleOKB::getDS1820Temp(response);

            LOG_INFO(QString("Sensor #%1 at line 2 temperature is %2").arg(sensorNumber).arg(temperature));
            addResponseParam(SystemState::TEMPERATURE, temperature);
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_PT100:
        {
            int sensorNumber = mCurrentTransaction.inputParams.value(SystemState::SENSOR_NUMBER).toInt();
            double temperature = ModuleOKB::getPT100Temp(response);

            LOG_INFO(QString("PT100 sensor %1 temperature is %2").arg(sensorNumber).arg(temperature));
            addResponseParam(SystemState::TEMPERATURE, temperature);
        }
        break;

    case ModuleCommands::START_MEASUREMENT_LINE_1:
    case ModuleCommands::START_MEASUREMENT_LINE_2:
    case ModuleCommands::RESET_LINE_1:
    case ModuleCommands::RESET_LINE_2:
        {
            // do nothing
        }
        break;

    default:
        LOG_WARNING(QString("Unexpected command id=0x%1 response received by OTD module (HEX): %2").arg(QString::number(command, 16)).arg(QString(response.toHex().toStdString().c_str())));
        return false;
        break;
    }

    return true;
}

void ModuleOTD::onModuleError()
{
    int TODO; //TODO here will be processing
}

void ModuleOTD::createResponse(Transaction& response)
{
    response = mCurrentTransaction;
}

void ModuleOTD::onApplicationFinish()
{
    closePort();
}
