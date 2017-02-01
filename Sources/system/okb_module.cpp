#include "Headers/system/okb_module.h"
#include "Headers/logger/Logger.h"
#include "Headers/system/system_state.h"

#include <QMap>
#include <QVariant>

ModuleOKB::ModuleOKB(QObject* parent):
    COMPortModule(parent),
      mAddress(0xff),
      mDefaultAddress(0xff)
{

}

ModuleOKB::~ModuleOKB()
{

}

bool ModuleOKB::postInit()
{
    if (!sendCommand(ModuleCommands::GET_MODULE_ADDRESS, 0, ModuleCommands::CURRENT))
    {
        return false;
    }

    if (!sendCommand(ModuleCommands::GET_MODULE_ADDRESS, 0, ModuleCommands::DEFAULT))
    {
        return false;
    }

    if (mAddress != mDefaultAddress)
    {
        emit incorrectSlot(mDefaultAddress);
    }

    return postInitOKBModule();
}

bool ModuleOKB::sendCommand(ModuleCommands::CommandID cmd, uint8_t param1, uint8_t param2, QByteArray* responseExt)
{
    QByteArray request(4, 0);
    request[0] = (cmd == ModuleCommands::GET_MODULE_ADDRESS) ? 0xff : mAddress;
    request[1] = cmd;
    request[2] = param1;
    request[3] = param2;

    QByteArray response;
    if (!COMPortModule::send(request, response))
    {
        return false;
    }

    if (responseExt)
    {
        *responseExt = response;
    }

    if (response.size() != 4) //TODO remove magic number
    {
        return false;
    }

    if (canReturnError(cmd) && response.at(3) == ModuleCommands::CMD_ERROR)
    {
        return false;
    }

    if (cmd == ModuleCommands::GET_MODULE_ADDRESS)
    {
        uint8_t value = response.at(2);

        if (param2 == ModuleCommands::DEFAULT)
        {
            mDefaultAddress = value;
        }
        else if (param2 == ModuleCommands::CURRENT)
        {
            mAddress = value;
        }
    }

    return true;
}

bool ModuleOKB::canReturnError(ModuleCommands::CommandID cmd) const
{
    switch (cmd)
    {
    case ModuleCommands::GET_MODULE_ADDRESS:
    case ModuleCommands::RESET_ERROR:
    case ModuleCommands::SOFT_RESET:
    case ModuleCommands::GET_SOWFTWARE_VER:
    case ModuleCommands::POWER_CHANNEL_CTRL:
    case ModuleCommands::SET_MKO_PWR_CHANNEL_STATE:
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
        return true; // can return ModuleCommands::ERROR in case of hardware failure

    case ModuleCommands::GET_STATUS_WORD:
    case ModuleCommands::ECHO:
    case ModuleCommands::GET_POWER_MODULE_STATE:
    case ModuleCommands::GET_PWR_MODULE_FUSE_STATE:
    case ModuleCommands::GET_CHANNEL_TELEMETRY:
    case ModuleCommands::GET_MKO_MODULE_STATE:
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

uint8_t ModuleOKB::defaultAddress() const
{
    return mDefaultAddress;
}

uint8_t ModuleOKB::currentAddress() const
{
    return mAddress;
}

void ModuleOKB::resetError()
{
    QByteArray requset(4, 0);

    requset[0] = mAddress;
    requset[1] = ModuleCommands::RESET_ERROR;
    requset[2] = 0x00;
    requset[3] = 0x00;

    QByteArray response;
    COMPortModule::send(requset, response);
    int TODO;
/*    if (readData.size() > 3)
    {
        return readData[3];
    }

    return 0;
    */
}

int ModuleOKB::softResetModule()
{
    //setActive(id, false);

    QByteArray buffer(4, 0);
    buffer[0] = mAddress;
    buffer[1] = ModuleCommands::SOFT_RESET;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    QByteArray readData1;
    COMPortModule::send(buffer, readData1);

    resetPort();
    //setActive(id, true);

    return readData1[3];
}

int ModuleOKB::getSoftwareVersion()
{
    QByteArray buffer(4, 0);
    buffer[0] = mAddress;
    buffer[1] = ModuleCommands::GET_SOWFTWARE_VER;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    QByteArray response;
    COMPortModule::send(buffer, response);
    return (response[2] * 10 + response[3]); // версия прошивки, ИМХО неправильно считается, т.к. два байта на нее
}

bool ModuleOKB::postInitOKBModule()
{
    return true;
}

bool ModuleOKB::hasErrors()
{
    QByteArray buffer(4, 0);
    buffer[0] = mAddress;
    buffer[1] = ModuleCommands::GET_STATUS_WORD;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    QByteArray response;
    if (COMPortModule::send(buffer, response))
    {
        bool hasError = false;
        uint8_t y = response[2];
        uint8_t x = response[3];

        if ((y & MODULE_READY_MASK) == 0)
        {
            LOG_ERROR("Module 0x%02x, is not ready", mAddress);
            hasError = true;
        }

        if ((y & HAS_ERRORS_MASK) > 0)
        {
            LOG_ERROR("Module 0x%02x, has errors", mAddress);
            hasError = true;
        }

        if ((y & AFTER_RESET_MASK) > 0)
        {
            LOG_ERROR("Module 0x%02x, is after RESET", mAddress);
            hasError = true;
        }

        if (x == 0x10)
        {
            LOG_ERROR("RS485 data byte lost in module 0x%02x due to buffer overflow", mAddress);
            hasError = true;
        }

        if (x == 0x11)
        {
            LOG_ERROR("UMART data byte lost in module 0x%02x due to buffer overflow", mAddress);
            hasError = true;
        }

        return hasError;
    }

    return true;
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
    case ModuleCommands::GET_SOWFTWARE_VER:
    case ModuleCommands::ECHO:
        {

        }
        break;

    default: // if command is not in "common" commands list
        {
            processCustomCommand(params, response);
        }
        break;
    }
    int TODO;


    if (false) // if it is common command (reset, reset error, sw version, echo, get module address) -> process here
    {

    }
    else // if it is custom comman type -> process by inherited module
    {

    }

    emit commandResult(response);
}
