#include "Headers/system/module.h"

#include <QtSerialPort>

namespace
{
    static const int WAIT_TIME = 100; // msec

}

Module::Module(QObject* parent):
    QObject(parent),
    mPort(Q_NULLPTR),
    mAddress(0xff),
    mDefaultAddress(0xff)
{
}

Module::~Module()
{

}

bool Module::send(const QByteArray& request, QByteArray& response)
{
    if (mPort && mPort->isOpen())
    {
        mPort->QIODevice::write(request);
        mPort->waitForBytesWritten(-1);

        response = mPort->readAll();
        while (mPort->waitForReadyRead(WAIT_TIME))
        {
            response.append(mPort->readAll());
        }

        if (response.size() != request.size())
        {
            qDebug("Incorrect response for command");
            return false;
        }

        return true;
    }

    return false;
}

void Module::setPort(QSerialPort* port)
{
    mPort = port;
}

bool Module::send(ModuleCommands::CommandID cmd, uint8_t param1, uint8_t param2)
{
    QByteArray request(4, 0);
    request[0] = mAddress;
    request[1] = cmd;
    request[2] = param1;
    request[3] = param2;

    QByteArray response;
    if (!send(request, response))
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

bool Module::canReturnError(ModuleCommands::CommandID cmd) const
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
        qDebug("Unknown command %i", cmd);
        break;
    }

    return false;
}

bool Module::init(QSerialPort* port)
{
    setPort(port);

    if (!send(ModuleCommands::GET_MODULE_ADDRESS, 0, ModuleCommands::CURRENT))
    {
        return false;
    }

    if (!send(ModuleCommands::GET_MODULE_ADDRESS, 0, ModuleCommands::DEFAULT))
    {
        return false;
    }

    if (mAddress != mDefaultAddress)
    {
        emit incorrectSlot(mDefaultAddress);
    }

    postInit();

    return true;
}

uint8_t Module::defaultAddress() const
{
    return mDefaultAddress;
}

uint8_t Module::currentAddress() const
{
    return mAddress;
}
