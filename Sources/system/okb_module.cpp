#include "Headers/system/okb_module.h"

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

    return true;
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
        qDebug("Unknown command %i", cmd);
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

QString ModuleOKB::checkStatusWord()
{
    int TODO;

    return "";

    /*if(isActive(TECH))
    {
        QString res;
        QByteArray buffer(4, 0);
        buffer[0] = TECH_DEFAULT_ADDR;
        buffer[1] = ModuleCommands::GET_STATUS_WORD;
        buffer[2] = 0x00;
        buffer[3] = 0x00;

        QByteArray readData1 = send(getPort(TECH), buffer);
        uint8_t x = readData1[2];
        uint8_t z1, z2, z3;
        z1 = x >> 7;
        z2 = x << 1;
        z2 = z2 >> 7;
        z3 = x << 2;
        z3 = z3 >> 7;
        if(z1 == 0)
            res += " Технол. модуль не готов к работе! \n";
        if(z2 == 1)
            res += " Ошибки у Технол. модуля! \n";
        if(z3 == 1)
            res += " Модуль Технол. после перезагрузки! \n";
        if(readData1.at(3)==0x10)
            res += " Потеря байта из-за переполнения буфера RS485! \n";
        return res;

        int TODO; // крайняя херна проверяется только у СТМ, у остальных есть только три предыдущих
    }

    return "";
    */
}

