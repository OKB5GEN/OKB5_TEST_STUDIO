#include "Headers/system/codecs/okb_codec.h"
#include "Headers/module_commands.h"

namespace
{
    static const int MESSAGE_SIZE = 4;
}

namespace OKBCodec
{
Message::Message():
    mAddress(0xFF),
    mCommandID(0x00),
    mData0(0x00),
    mData1(0x00)
{
}

Message::Message(const QByteArray& encodedBuffer)
{
    if (encodedBuffer.size() == MESSAGE_SIZE)
    {
        mAddress = encodedBuffer[0];
        mCommandID = encodedBuffer[1];
        mData0 = encodedBuffer[2];
        mData1 = encodedBuffer[3];
    }
}

void Message::setModuleAddress(uint8_t address)
{
    mAddress = address;
}

uint8_t Message::moduleAddress() const
{
    return mAddress;
}

QByteArray Message::encode() const
{
    QByteArray buffer;
    encode(buffer);
    return buffer;
}

void Message::encode(QByteArray& outBuffer) const
{
    outBuffer.append(mAddress);
    outBuffer.append(mCommandID);
    outBuffer.append(mData0);
    outBuffer.append(mData1);
}

OKErrorMessage::OKErrorMessage(QByteArray& encodedBuffer):
    Message(encodedBuffer)
{

}

bool OKErrorMessage::hasError() const
{
    return (mData1 == COMMAND_ERROR);
}

GetModuleAddressRequest::GetModuleAddressRequest(Type type)
{
    mAddress = 0xff;
    mCommandID = ModuleCommands::GET_MODULE_ADDRESS;
    mData0 = 0x00;
    mData1 = type;
}

GetModuleAddressResponse::GetModuleAddressResponse(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{
}

uint8_t GetModuleAddressResponse::moduleAddress() const
{
    return mData0;
}


GetStatusWordRequest::GetStatusWordRequest(uint8_t moduleAddress)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::GET_STATUS_WORD;
    mData0 = 0x00;
    mData1 = 0x00;
}

GetStatusWordResponse::GetStatusWordResponse(QByteArray& encodedBuffer):
    Message(encodedBuffer)
{
}

bool GetStatusWordResponse::isReady() const
{
    return ((mData0 & MODULE_READY_MASK) > 0);
}

bool GetStatusWordResponse::hasError() const
{
    return ((mData0 & HAS_ERRORS_MASK) > 0);
}

bool GetStatusWordResponse::isAfterReset() const
{
    return ((mData0 & AFTER_RESET_MASK) > 0);
}

uint8_t GetStatusWordResponse::errorCode() const
{
    return mData1;
}

ResetErrorRequest::ResetErrorRequest(uint8_t moduleAddress)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::RESET_ERROR;
    mData0 = 0;
    mData1 = 0;
}

ResetErrorResponse::ResetErrorResponse(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{

}

SoftResetRequest::SoftResetRequest(uint8_t moduleAddress)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::SOFT_RESET;
    mData0 = 0;
    mData1 = 0;
}

SoftResetResponse::SoftResetResponse(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{

}

GetSoftwareVersionRequest::GetSoftwareVersionRequest(uint8_t moduleAddress)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::GET_SOWFTWARE_VERSION;
    mData0 = 0;
    mData1 = 0;
}

GetSoftwareVersionResponse::GetSoftwareVersionResponse(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{

}

uint8_t GetSoftwareVersionResponse::majorVersion() const
{
    return mData0;
}

uint8_t GetSoftwareVersionResponse::minorVersion() const
{
    return mData1;
}

EchoRequest::EchoRequest(uint8_t moduleAddress, uint8_t data0, uint8_t data1)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::ECHO;
    mData0 = data0;
    mData1 = data1;
}

EchoResponse::EchoResponse(QByteArray& encodedBuffer):
    Message(encodedBuffer)
{

}

uint8_t EchoResponse::data0() const
{
    return mData0;
}

uint8_t EchoResponse::data1() const
{
    return mData1;
}

SetPowerSupplyChannelStateRequest::SetPowerSupplyChannelStateRequest(uint8_t moduleAddress, ChannelID channel, RelayState state)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::SET_POWER_CHANNEL_STATE;
    mData0 = channel;
    mData1 = state;
}

SetPowerSupplyChannelStateResponse::SetPowerSupplyChannelStateResponse(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{

}

GetPowerSupplyChannelStateRequest::GetPowerSupplyChannelStateRequest(uint8_t moduleAddress, ChannelID channel)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::GET_POWER_CHANNEL_STATE;
    mData0 = channel;
    mData1 = 0;
}

GetPowerSupplyChannelStateResponse::GetPowerSupplyChannelStateResponse(QByteArray& encodedBuffer):
    Message(encodedBuffer)
{

}

Message::ChannelID GetPowerSupplyChannelStateResponse::channel() const
{
    return Message::ChannelID(mData0);
}

Message::RelayState GetPowerSupplyChannelStateResponse::state() const
{
    return Message::RelayState(mData1);
}

GetPowerSupplyFuseStateRequest::GetPowerSupplyFuseStateRequest(uint8_t moduleAddress, FuseID fuseID)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::GET_FUSE_STATE;
    mData0 = fuseID;
    mData1 = 0;
}

GetPowerSupplyFuseStateResponse::GetPowerSupplyFuseStateResponse(QByteArray& encodedBuffer):
    Message(encodedBuffer)
{

}

Message::FuseID GetPowerSupplyFuseStateResponse::fuseID() const
{
    return FuseID(mData0);
}

Message::FuseState GetPowerSupplyFuseStateResponse::fuseState() const
{
    return FuseState(mData1);
}

GetChannelTelemetryRequest::GetChannelTelemetryRequest(uint8_t moduleAddress, int channelID)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::GET_CHANNEL_TELEMETRY;
    mData0 = channelID;
    mData1 = 0;
}

GetChannelTelemetryResponse::GetChannelTelemetryResponse(QByteArray& encodedBuffer):
    Message(encodedBuffer)
{

}

qreal GetChannelTelemetryResponse::voltage() const
{
    qreal voltage = qreal((mData0 << 8) | mData1) / 10000;
    return voltage;
}

SetMKOPowerSupplyChannelStateRequest::SetMKOPowerSupplyChannelStateRequest(uint8_t moduleAddress, MKOChannelID channel, RelayState state)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::SET_MKO_POWER_CHANNEL_STATE;
    mData0 = channel;
    mData1 = state;
}

SetMKOPowerSupplyChannelStateResponse::SetMKOPowerSupplyChannelStateResponse(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{

}

GetMKOPowerSupplyChannelStateRequest::GetMKOPowerSupplyChannelStateRequest(uint8_t moduleAddress, MKOChannelID channel)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::GET_MKO_POWER_CHANNEL_STATE;
    mData0 = channel;
    mData1 = 0;
}

GetMKOPowerSupplyChannelStateResponse::GetMKOPowerSupplyChannelStateResponse(QByteArray& encodedBuffer):
    Message(encodedBuffer)
{

}

Message::MKOChannelID GetMKOPowerSupplyChannelStateResponse::channel() const
{
    return MKOChannelID(mData0);
}

Message::RelayState GetMKOPowerSupplyChannelStateResponse::state() const
{
    return RelayState(mData1);
}

GetPT100TemperatureRequest::GetPT100TemperatureRequest(uint8_t moduleAddress, int sensorID)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::GET_TEMPERATURE_PT100;
    mData0 = sensorID;
    mData1 = 0;
}

GetPT100TemperatureResponse::GetPT100TemperatureResponse(QByteArray& encodedBuffer):
    Message(encodedBuffer)
{

}

qreal GetPT100TemperatureResponse::temperature() const
{
    double temperature = (mData0 << 8) | mData1;
    temperature = temperature / 32 - 256;
    return temperature;

    //int TODO; // parse error
    //x = x / 100;
    //y = y / 100;
    //if(x == -256) ui->OTDerror->setText("Ошибка измерения датчика");
    //if(y == -256) ui->OTDerror->setText("Ошибка измерения датчика");
    //if(x > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");
    //if(y > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");

    //Формула вычесления температуры: ADCcode = YX  unsigned int ADCcode; Temp = ((float)ADCcode / 32) - 256; Если ADCcode = 0x0000 - ошибка измерения Если ADCcode = 0xFFFF - ошибка обращения к модулю
}

bool GetPT100TemperatureResponse::hasMeasurementError() const
{
    return (mData0 == 0x00 && mData1 == 0x00) || (mData0 == 0xFF && mData1 == 0xFF);
}

GetSensorsCountLine1Request::GetSensorsCountLine1Request(uint8_t moduleAddress)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::GET_DS1820_COUNT_LINE_1;
    mData0 = 0;
    mData1 = 0;
}

GetSensorsCountLine1Response::GetSensorsCountLine1Response(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{

}

int GetSensorsCountLine1Response::count() const
{
    return mData0;
}

GetSensorsCountLine2Request::GetSensorsCountLine2Request(uint8_t moduleAddress)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::GET_DS1820_COUNT_LINE_2;
    mData0 = 0;
    mData1 = 0;
}

GetSensorsCountLine2Response::GetSensorsCountLine2Response(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{

}

int GetSensorsCountLine2Response::count() const
{
    return mData1;
}

ResetLine1Request::ResetLine1Request(uint8_t moduleAddress)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::RESET_LINE_1;
    mData0 = 0;
    mData1 = 0;
}

ResetLine1Response::ResetLine1Response(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{

}

ResetLine2Request::ResetLine2Request(uint8_t moduleAddress)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::RESET_LINE_2;
    mData0 = 0;
    mData1 = 0;
}

ResetLine2Response::ResetLine2Response(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{

}

StartMeasurementLine1Request::StartMeasurementLine1Request(uint8_t moduleAddress)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::START_MEASUREMENT_LINE_1;
    mData0 = 0;
    mData1 = 0;
}

StartMeasurementLine1Response::StartMeasurementLine1Response(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{

}

StartMeasurementLine2Request::StartMeasurementLine2Request(uint8_t moduleAddress)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::START_MEASUREMENT_LINE_2;
    mData0 = 0;
    mData1 = 0;
}

StartMeasurementLine2Response::StartMeasurementLine2Response(QByteArray& encodedBuffer):
    OKErrorMessage(encodedBuffer)
{

}

GetDS1820TemperatureLine1Request::GetDS1820TemperatureLine1Request(uint8_t moduleAddress, int sensorID)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1;
    mData0 = sensorID;
    mData1 = 0;
}

GetDS1820TemperatureLine1Response::GetDS1820TemperatureLine1Response(QByteArray& encodedBuffer):
    Message(encodedBuffer)
{

}

qreal GetDS1820TemperatureLine1Response::temperature() const
{
    uint8_t z;
    double uu = (mData0 << 8) | mData1;
    uint8_t x = mData0;
    z = x << 4;
    z = z >> 7;
    if (z == 0) // знаковый бит, положительная температура
    {
        uu = uu / 16;
    }

    if (z == 1) // знаковый бит, отрицательная температура
    {
        uu = (uu - 4096) / 16;
    }

    return uu;
}

GetDS1820TemperatureLine2Request::GetDS1820TemperatureLine2Request(uint8_t moduleAddress, int sensorID)
{
    mAddress = moduleAddress;
    mCommandID = ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2;
    mData0 = sensorID;
    mData1 = 0;
}

GetDS1820TemperatureLine2Response::GetDS1820TemperatureLine2Response(QByteArray& encodedBuffer):
    Message(encodedBuffer)
{

}

qreal GetDS1820TemperatureLine2Response::temperature() const
{
    uint8_t z;
    double uu = (mData0 << 8) | mData1;
    uint8_t x = mData0;
    z = x << 4;
    z = z >> 7;
    if (z == 0) // знаковый бит, положительная температура
    {
        uu = uu / 16;
    }

    if (z == 1) // знаковый бит, отрицательная температура
    {
        uu = (uu - 4096) / 16;
    }

    return uu;
}

}
