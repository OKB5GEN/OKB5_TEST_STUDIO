#include "Headers/system/okb_module.h"
#include "Headers/logger/Logger.h"
#include "Headers/system/system_state.h"

namespace
{
    static const int OKB_MODULE_MESSAGE_SIZE = 4; // all OKB modules requests/responses have 4-byte length
}

ModuleOKB::ModuleOKB(QObject* parent):
    COMPortModule(parent),
    mCurrentAddress(0xff),
    mDefaultAddress(0xff)
{
    int TODO; // вообще надо проверять ОКБ модули только по типу и идентифицировать модуль запросом адреса модуля
}

ModuleOKB::~ModuleOKB()
{

}

void ModuleOKB::addModuleCmd(ModuleCommands::CommandID cmd, uint8_t param1, uint8_t param2)
{
    QByteArray request;
    request.append((cmd == ModuleCommands::GET_MODULE_ADDRESS) ? 0xff : mCurrentAddress);
    request.append(cmd);
    request.append(param1);
    request.append(param2);

    addRequest(cmd, request);
}

bool ModuleOKB::canReturnError(ModuleCommands::CommandID cmd) const
{
    switch (cmd)
    {
    case ModuleCommands::GET_MODULE_ADDRESS:
    case ModuleCommands::RESET_ERROR:
    case ModuleCommands::SOFT_RESET:
    case ModuleCommands::GET_SOWFTWARE_VERSION:
    case ModuleCommands::SET_POWER_CHANNEL_STATE:
    case ModuleCommands::SET_MKO_POWER_CHANNEL_STATE:
    case ModuleCommands::SET_PACKET_SIZE_CAN:
    case ModuleCommands::ADD_BYTES_CAN:
    case ModuleCommands::SEND_PACKET_CAN:
    case ModuleCommands::CLEAN_BUFFER_CAN:
    case ModuleCommands::SET_TECH_INTERFACE:
    case ModuleCommands::GET_TECH_INTERFACE:
    case ModuleCommands::SET_MODE_RS485:
    case ModuleCommands::SET_SPEED_RS485:
    case ModuleCommands::SET_PACKET_SIZE_RS485:
    case ModuleCommands::ADD_BYTES_RS485:
    case ModuleCommands::SEND_PACKET_RS485:
    case ModuleCommands::CLEAN_BUFFER_RS485:
    case ModuleCommands::GET_DS1820_COUNT_LINE_1:
    case ModuleCommands::GET_DS1820_COUNT_LINE_2:
    case ModuleCommands::RESET_LINE_1:
    case ModuleCommands::RESET_LINE_2:
    case ModuleCommands::START_MEASUREMENT_LINE_1:
    case ModuleCommands::START_MEASUREMENT_LINE_2:
    case ModuleCommands::GET_DS1820_ADDR_LINE_1:
    case ModuleCommands::GET_DS1820_ADDR_LINE_2:
    case ModuleCommands::GET_POWER_CHANNEL_STATE:
    case ModuleCommands::GET_FUSE_STATE:
    case ModuleCommands::GET_MKO_POWER_CHANNEL_STATE:
        return true; // can return ModuleCommands::ERROR in case of hardware failure/errors

    case ModuleCommands::GET_STATUS_WORD:
    case ModuleCommands::ECHO:
    case ModuleCommands::GET_CHANNEL_TELEMETRY:
    case ModuleCommands::CHECK_RECV_DATA_CAN:
    case ModuleCommands::RECV_DATA_CAN:
    case ModuleCommands::CHECK_RECV_DATA_RS485:
    case ModuleCommands::RECV_DATA_RS485:
    case ModuleCommands::RECV_DATA_SSI:
    case ModuleCommands::GET_TEMPERATURE_PT100:
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
        return false; // always return some data need to be analyzed

    default:
        LOG_ERROR(QString("Unknown command %1").arg(cmd));
        break;
    }

    return false;
}

void ModuleOKB::processCommand(const Transaction& request)
{
    mCurrentTransaction.clear();
    mCurrentTransaction = request;

    ModuleCommands::CommandID command = ModuleCommands::CommandID(mCurrentTransaction.commandID);

    switch (command)
    {
    case ModuleCommands::GET_MODULE_ADDRESS:
        {
            int addressType = mCurrentTransaction.inputParams.value(SystemState::MODULE_ADDRESS).toInt();
            addModuleCmd(command, 0, addressType);
        }
        break;
    case ModuleCommands::GET_STATUS_WORD:
    case ModuleCommands::RESET_ERROR:
    case ModuleCommands::SOFT_RESET: //TODO command not added ti GUI
    case ModuleCommands::GET_SOWFTWARE_VERSION: //TODO command not added ti GUI
        {
            addModuleCmd(command, 0, 0);
        }
        break;

    case ModuleCommands::ECHO:
        {
            //TODO command is not added to GUI
            int data1 = mCurrentTransaction.inputParams.value(SystemState::ECHO_DATA_1, 0).toInt();
            int data2 = mCurrentTransaction.inputParams.value(SystemState::ECHO_DATA_2, 0).toInt();
            addModuleCmd(command, data1, data2);
        }
        break;

    default: // if command is not in "common" commands list, process it by inherited module
        processCustomCommand();
        break;
    }
}

bool ModuleOKB::processResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response)
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(operationID);
    QString error;

    if (response.size() != OKB_MODULE_MESSAGE_SIZE)
    {
        error = QString("Malformed response packet. Size=%1").arg(response.size());
    }
    else if (canReturnError(command) && response.at(3) == ModuleCommands::CMD_ERROR)
    {
        error = QString("Command %1 execution finished with error").arg(command);
    }

    if (!error.isEmpty())
    {
        LOG_ERROR(error);
        onModuleError();
        return false;
    }

    switch (command)
    {
    case ModuleCommands::GET_MODULE_ADDRESS:
        {
            //TODO QString error = QString("Module is in incorrect slot. Current address=0x%1, Default address=0x%2").arg(QString::number(mCurrentAddress, 16)).arg(QString::number(mDefaultAddress, 16));
            ModuleCommands::ModuleAddress address = ModuleCommands::ModuleAddress(request.at(3));
            uint8_t value = response.at(2);

            if (address == ModuleCommands::CURRENT)
            {
                mCurrentAddress = value;
                LOG_INFO(QString("Module current address is 0x%1").arg(QString::number(mCurrentAddress, 16)));
            }
            else if (address == ModuleCommands::DEFAULT)
            {
                mDefaultAddress = value;
                LOG_INFO(QString("Module default address is 0x%1").arg(QString::number(mDefaultAddress, 16)));
            }

            addResponseParam(SystemState::MODULE_ADDRESS, value);
        }
        break;

    case ModuleCommands::GET_STATUS_WORD:
        {
            QString error;
            uint8_t y = response[2];
            uint8_t x = response[3];

            if ((y & MODULE_READY_MASK) == 0)
            {
                error = QString("Module 0x%1, is not ready").arg(QString::number(mCurrentAddress, 16));
                addResponseParam(SystemState::MODULE_READY, 1);
            }
            else
            {
                addResponseParam(SystemState::MODULE_READY, 0);
            }

            if ((y & HAS_ERRORS_MASK) > 0)
            {
                error = QString("Module 0x%1, has errors").arg(QString::number(mCurrentAddress, 16));
                addResponseParam(SystemState::MODULE_HAS_ERRORS, 1);
            }
            else
            {
                addResponseParam(SystemState::MODULE_HAS_ERRORS, 0);
            }

            if ((y & AFTER_RESET_MASK) > 0)
            {
                LOG_WARNING(QString("Module 0x%1, is after RESET").arg(QString::number(mCurrentAddress, 16)));
                addResponseParam(SystemState::MODULE_AFTER_RESET, 1);
            }
            else
            {
                addResponseParam(SystemState::MODULE_AFTER_RESET, 0);
            }

            if (x == 0x10)
            {
                LOG_WARNING(QString("RS485 data byte lost in module 0x%1 due to buffer overflow").arg(QString::number(mCurrentAddress, 16)));//TODO is it error?
            }

            if (x == 0x11)
            {
                LOG_WARNING(QString("UMART data byte lost in module 0x%1 due to buffer overflow").arg(QString::number(mCurrentAddress, 16)));//TODO is it error?
            }

            mCurrentTransaction.error = error;
        }
        break;

    case ModuleCommands::SOFT_RESET:
        {
            int TODO;// при софт ресете СТМ все его реле отрубаются поэтому надо софт ресет обрабатывать весьма хитрожопо (пинать МКО например, потому что он перезагрузится по факту)

            // TODO: remote module will be reset by watchdog, need to close and reset COM port itself and re-initialize it
            softReset();
        }
        break;

    case ModuleCommands::RESET_ERROR:
        {
            int TODO; // need some processing?
        }
        break;

    case ModuleCommands::GET_SOWFTWARE_VERSION:
        {
            int version = 0; //"response[2].response[3]" типа "0.3" //TODO
        }
        break;

    case ModuleCommands::ECHO:
        {
            int TODO; // check 2 and 3 bytes of request and response
        }
        break;

    default: // if command is not in "common" commands list, redirect processing to inherited class
        return processCustomResponse(operationID, request, response);
        break;
    }

    return true;
}

void ModuleOKB::onTransmissionError(uint32_t operationID)
{
    onModuleError();
}

void ModuleOKB::onTransmissionComplete()
{
    Transaction response;
    createResponse(response);

    LOG_INFO(QString("Send response to cyclogram"));
    emit commandResult(response);
}

void ModuleOKB::createResponse(Transaction& response)
{
    LOG_WARNING(QString("ModuleOKB::createResponse not reimplemented"));
    int TODO; // do nothing?
}
