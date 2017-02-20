#include "Headers/system/okb_module.h"
#include "Headers/logger/Logger.h"
#include "Headers/system/system_state.h"

#include <QMap>
#include <QVariant>

namespace
{
    static const int OKB_MODULE_MESSAGE_SIZE = 4; // all OKB modules requests/responses have 4-byte length
}

ModuleOKB::ModuleOKB(QObject* parent):
    COMPortModule(parent),
    mCurrentAddress(0xff),
    mDefaultAddress(0xff),
    mCommonInitializationFinished(false),
    mCustomInitializationFinished(false)
{
    int TODO; // вообще надо проверять ОКБ модули только по типу и идентифицировать модуль запросом адреса модуля
}

ModuleOKB::~ModuleOKB()
{

}

void ModuleOKB::initializeCustom()
{
    // OKB Module initialization process:
    //  1. Ask module for its default address
    //  2. Ask module for its current address
    //  3. Compare module default and current addresses
    //  4. If addresses are not equal - send initializationFinished signal with ERROR and abort further initialization
    //  5. If addresses are equal - ask module for its status word to check errors
    //  6. If module has errors - send initializationFinished signal with ERROR and abort further initialization
    //  7. If module has no errors - begin module custom initialization, by calling initializeCustomOKBModule()
    //  8. If initializeCustomOKBModule() not overridden it just will send initializationFinished SUCCESS signal without error
    //  9. Otherwise initializationFinished signal must be sent from inherited class (SUCCES or ERROR depending on custom logic)
    // 10. If some transmission error will happen during initialization, we consider it as critical error and abort initialization

    // Initialization step 1: Aquire and check module addresses
    addModuleCmd(ModuleCommands::GET_MODULE_ADDRESS, 0, ModuleCommands::DEFAULT);
    addModuleCmd(ModuleCommands::GET_MODULE_ADDRESS, 0, ModuleCommands::CURRENT);
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
        return true; // can return ModuleCommands::ERROR in case of hardware failure/errors

    case ModuleCommands::GET_STATUS_WORD:
    case ModuleCommands::ECHO:
    case ModuleCommands::GET_POWER_CHANNEL_STATE:
    case ModuleCommands::GET_FUSE_STATE:
    case ModuleCommands::GET_CHANNEL_TELEMETRY:
    case ModuleCommands::GET_MKO_POWER_CHANNEL_STATE:
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
        LOG_ERROR("Unknown command %i", cmd);
        break;
    }

    return false;
}

void ModuleOKB::initializeCustomOKBModule()
{
    //TODO stub here
    mModuleReady = true;
    mCustomInitializationFinished = true;
    emit initializationFinished(QString(""));
}

void ModuleOKB::processCommand(const QMap<uint32_t, QVariant>& params)
{
    QMap<uint32_t, QVariant> response;

    ModuleCommands::CommandID command = ModuleCommands::CommandID(params.value(SystemState::COMMAND_ID).toUInt());

    response[SystemState::MODULE_ID] = params.value(SystemState::MODULE_ID);
    response[SystemState::COMMAND_ID] = QVariant(uint32_t(command));
    response[SystemState::ERROR_CODE] = QVariant(uint32_t(0));
    response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);

    switch (command)
    {
    case ModuleCommands::GET_MODULE_ADDRESS:
    case ModuleCommands::GET_STATUS_WORD:
    case ModuleCommands::RESET_ERROR:
    case ModuleCommands::SOFT_RESET:
    case ModuleCommands::GET_SOWFTWARE_VERSION:
    case ModuleCommands::ECHO:
        {
            int TODO; // decode and process common commands here
        }
        break;

    default: // if command is not in "common" commands list, process it by inherited module
        processCustomCommand(params, response);
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
        if (!mCommonInitializationFinished)
        {
            emit initializationFinished(error);
        }
        else
        {
            LOG_ERROR(error);
            onModuleError();
        }

        return false;
    }

    switch (command)
    {
    case ModuleCommands::GET_MODULE_ADDRESS:
        {
            ModuleCommands::ModuleAddress address = ModuleCommands::ModuleAddress(request.at(3));
            uint8_t value = response.at(2);

            if (address == ModuleCommands::CURRENT)
            {
                mCurrentAddress = value;
                LOG_INFO("Module current address is 0x%x", mCurrentAddress);
            }
            else if (address == ModuleCommands::DEFAULT)
            {
                mDefaultAddress = value;
                LOG_INFO("Module default address is 0x%x", mDefaultAddress);
            }

            // Initialization step 1 finish >>>
            if (mCurrentAddress != 0xff && mDefaultAddress != 0xff && !mCommonInitializationFinished)
            {
                if (mCurrentAddress != mDefaultAddress)
                {
                    QString error = QString("Module is in incorrect slot. Current address=0x%1, Default address=0x%2").arg(QString::number(mCurrentAddress, 16)).arg(QString::number(mDefaultAddress, 16));
                    emit initializationFinished(error);
                }
                else
                {
                    // Initialization step 2: Check module status word
                    addModuleCmd(ModuleCommands::GET_STATUS_WORD, 0, 0);
                }
            }
        }
        break;

    case ModuleCommands::GET_STATUS_WORD:
        {
            int TODO; // are all of these values are errors?

            QString error;
            uint8_t y = response[2];
            uint8_t x = response[3];

            if ((y & MODULE_READY_MASK) == 0)
            {
                error = QString("Module 0x%1, is not ready").arg(QString::number(mCurrentAddress, 16));
                //TODO soft reset module?
            }

            if ((y & HAS_ERRORS_MASK) > 0)
            {
                error = QString("Module 0x%1, has errors").arg(QString::number(mCurrentAddress, 16));
            }

            if ((y & AFTER_RESET_MASK) > 0)
            {
                LOG_WARNING("Module 0x%02x, is after RESET", mCurrentAddress);//TODO is it error?
            }

            if (x == 0x10)
            {
                LOG_WARNING("RS485 data byte lost in module 0x%02x due to buffer overflow", mCurrentAddress);//TODO is it error?
            }

            if (x == 0x11)
            {
                LOG_WARNING("UMART data byte lost in module 0x%02x due to buffer overflow", mCurrentAddress);//TODO is it error?
            }

            // Initialization step 2 finish >>>
            if (!mCommonInitializationFinished)
            {
                if (!error.isEmpty())
                {
                    emit initializationFinished(error);
                }
                else
                {
                    mCommonInitializationFinished = true;
                    initializeCustomOKBModule();
                }
            }
            else
            {
                if (!error.isEmpty())
                {
                    LOG_ERROR(error);
                }
            }
        }
        break;

    case ModuleCommands::SOFT_RESET:
        {
            // remote module will be reset by watchdog, need to close and reset COM port itself and re-initialize it
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
    QMap<uint32_t, QVariant> response;
    createResponse(response);

    if (!response.empty())
    {
        LOG_INFO("Send response to cyclogram");
        emit commandResult(response);
    }
}

void ModuleOKB::onSoftResetComplete()
{
    int TODO; // QSerialPort is up, need some module re-initialization will be here such as initializeCustom() call
}

void ModuleOKB::createResponse(QMap<uint32_t, QVariant>& response)
{
    int TODO; // do nothing?
}
