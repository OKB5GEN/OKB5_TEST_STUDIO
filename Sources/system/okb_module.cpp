#include "Headers/system/okb_module.h"
#include "Headers/logger/Logger.h"
#include "Headers/system/system_state.h"

#include <QMap>
#include <QVariant>

namespace
{
    static const int PACKET_SIZE = 4; // all OKB modules commands have 4-byte length
}

ModuleOKB::ModuleOKB(QObject* parent):
    COMPortModule(parent),
    mCurrentAddress(0xff),
    mDefaultAddress(0xff),
    mCommonInitializationFinished(false)
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

    addCommandToQueue(ModuleCommands::GET_MODULE_ADDRESS, 0, ModuleCommands::DEFAULT);
    addCommandToQueue(ModuleCommands::GET_MODULE_ADDRESS, 0, ModuleCommands::CURRENT);
    addCommandToQueue(ModuleCommands::GET_STATUS_WORD, 0, 0);
    processQueue();
}

void ModuleOKB::addCommandToQueue(ModuleCommands::CommandID cmd, uint8_t param1, uint8_t param2)
{
    Request request;

    request.data.append((cmd == ModuleCommands::GET_MODULE_ADDRESS) ? 0xff : mCurrentAddress);
    request.data.append(cmd);
    request.data.append(param1);
    request.data.append(param2);

    request.operation = cmd;

    mRequestQueue.push_back(request);
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

void ModuleOKB::resetError()
{
    int TODO1; // not tested
    QByteArray requset(4, 0);

    requset[0] = mCurrentAddress;
    requset[1] = ModuleCommands::RESET_ERROR;
    requset[2] = 0x00;
    requset[3] = 0x00;

    send(requset);
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
    int TODO; // not tested
    //setActive(id, false);

    QByteArray buffer(4, 0);
    buffer[0] = mCurrentAddress;
    buffer[1] = ModuleCommands::SOFT_RESET;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    send(buffer);

    resetPort();
    //setActive(id, true);

    return 0; //readData1[3]; //TODO
}

int ModuleOKB::getSoftwareVersion()
{
    QByteArray buffer(4, 0);
    buffer[0] = mCurrentAddress;
    buffer[1] = ModuleCommands::GET_SOWFTWARE_VER;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    send(buffer);
    return 0; //TODO (response[2] * 10 + response[3]); // версия прошивки, ИМХО неправильно считается, т.к. два байта на нее
}

void ModuleOKB::initializeCustomOKBModule()
{
    emit initializationFinished(QString(""));
}

void ModuleOKB::onExecutionError()
{
    //TODO here will be processing
}

bool ModuleOKB::hasErrors()
{
    QByteArray buffer(4, 0);
    buffer[0] = mCurrentAddress;
    buffer[1] = ModuleCommands::GET_STATUS_WORD;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    if (send(buffer))
    {
        return false;//TODO
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
            int TODO; // process commands here
        }
        break;

    default: // if command is not in "common" commands list
        {
            processCustomCommand(params, response);
        }
        break;
    }

    processQueue();
    //emit commandResult(response);
}

void ModuleOKB::processResponse(const QByteArray& response)
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(mRequestQueue.front().operation);

    QString error;

    if (response.isEmpty())
    {
        error = QString("No response received by module!");
    }
    else if (response.size() != PACKET_SIZE)
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
            onExecutionError();
        }

        LOG_ERROR("Flushing module request queue...");
        mRequestQueue.clear();
        return;
    }

    switch (command)
    {
    case ModuleCommands::GET_MODULE_ADDRESS:
        {
            ModuleCommands::ModuleAddress address = ModuleCommands::ModuleAddress(mRequestQueue.front().data.at(3));
            uint8_t value = response.at(2);

            if (address == ModuleCommands::CURRENT)
            {
                mCurrentAddress = value;
            }
            else if (address == ModuleCommands::DEFAULT)
            {
                mDefaultAddress = value;
            }

            if (   mCurrentAddress != 0xff
                && mDefaultAddress != 0xff
                && !mCommonInitializationFinished
                && mCurrentAddress != mDefaultAddress)
            {
                QString error = QString("Module is in incorrect slot. Current address=0x%1, Default address=0x%2").arg(QString::number(mCurrentAddress, 16)).arg(QString::number(mDefaultAddress, 16));
                emit initializationFinished(error);
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

            if (!mCommonInitializationFinished) // application initialization
            {
                if (!error.isEmpty())
                {
                    int TODO; // possibly try to reset error?
                    QString error = QString("Module has errors");
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

    case ModuleCommands::RESET_ERROR:
    case ModuleCommands::SOFT_RESET:
    case ModuleCommands::GET_SOWFTWARE_VER:
    case ModuleCommands::ECHO:
        {
            int TODO; // process commands here
        }
        break;

    default: // if command is not in "common" commands list
        {
            processCustomResponse(response);
        }
        break;
    }

    if (!mRequestQueue.front().response.empty())
    {
        emit commandResult(mRequestQueue.front().response);
    }

    mRequestQueue.pop_front();
    processQueue();
}
