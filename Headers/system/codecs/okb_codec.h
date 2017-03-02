#ifndef OKB_CODEC_H
#define OKB_CODEC_H

#include <QByteArray>

namespace OKBCodec
{
//===== Base Message
class Message
{
public:
    enum ChannelID
    {
        BUP_MAIN      = 0x01,
        BUP_RESERVE   = 0x02,
        UNKNOWN_3     = 0x03, // reserved
        HEATER_LINE_1 = 0x04,
        HEATER_LINE_2 = 0x05,
        DRIVE_CONTROL = 0x06,
    };

    Q_ENUM(ChannelID);

    enum RelayState
    {
        POWER_OFF = 0x00,
        POWER_ON  = 0x01,
    };

    Q_ENUM(RelayState);

    enum FuseState
    {
        OK      = 0x00,
        FAULTY  = 0x01,
    };

    Q_ENUM(FuseState);

    enum FuseID
    {
        FUSE_1_1 = 0x01,
        FUSE_1_2 = 0x02,
        FUSE_1_3 = 0x03,
        FUSE_1_4 = 0x04,
        FUSE_2_1 = 0x05,
        FUSE_2_2 = 0x06,
        FUSE_2_3 = 0x07,
        FUSE_2_4 = 0x08,
    };

    Q_ENUM(FuseID);

    enum MKOChannelID
    {
        MKO_1 = 0x01,
        MKO_2 = 0x02,
        MKO_3 = 0x03,
        MKO_4 = 0x04
    };

    enum
    {
        COMMAND_ERROR = 0x02
    };

    Q_ENUM(MKOChannelID);

    Message();
    Message(const QByteArray& encodedBuffer);

    void setModuleAddress(uint8_t address);
    uint8_t moduleAddress() const;

    QByteArray encode() const;
    void encode(QByteArray& outBuffer) const;

protected:
    uint8_t mAddress;
    uint8_t mCommandID;
    uint8_t mData0;
    uint8_t mData1;
};

class OKErrorMessage: public Message
{
public:
    OKErrorMessage(QByteArray& encodedBuffer);
    bool hasError() const;
};

//===== Get Module Address =====
class GetModuleAddressRequest: public Message
{
public:
    enum Type
    {
        CURRENT = 0x01,
        DEFAULT = 0x02
    };

    GetModuleAddressRequest(Type type);
};

class GetModuleAddressResponse: public OKErrorMessage
{
public:
    GetModuleAddressResponse(QByteArray& encodedBuffer);
    uint8_t moduleAddress() const;
};

//===== Get Status Word =====
class GetStatusWordRequest: public Message
{
public:
    GetStatusWordRequest(uint8_t moduleAddress);
};

class GetStatusWordResponse: public Message
{
public:
    enum StatusMask
    {
        MODULE_READY_MASK = 0x80,
        HAS_ERRORS_MASK   = 0x40,
        AFTER_RESET_MASK  = 0x20,
    };

    enum ErrorCodes
    {
        RS485_BYTE_LOST_DUE_TO_BUF_OVERFLOW = 0x10,
        UMART_BYTE_LOST_DUE_TO_BUF_OVERFLOW = 0x11,
    };

    GetStatusWordResponse(QByteArray& encodedBuffer);

    bool isReady() const;
    bool hasError() const;
    bool isAfterReset() const;
    uint8_t errorCode() const;
};

//===== Reset Error =====
class ResetErrorRequest: public Message
{
public:
    ResetErrorRequest(uint8_t moduleAddress);
};

class ResetErrorResponse: public OKErrorMessage
{
public:
    ResetErrorResponse(QByteArray& encodedBuffer);
};

//===== Soft Reset =====
class SoftResetRequest: public Message
{
public:
    SoftResetRequest(uint8_t moduleAddress);
};

class SoftResetResponse: public OKErrorMessage
{
public:
    SoftResetResponse(QByteArray& encodedBuffer);
};

//===== Get Software Version =====
class GetSoftwareVersionRequest: public Message
{
public:
    GetSoftwareVersionRequest(uint8_t moduleAddress);
};

class GetSoftwareVersionResponse: public OKErrorMessage
{
public:
    GetSoftwareVersionResponse(QByteArray& encodedBuffer);
    uint8_t majorVersion() const;
    uint8_t minorVersion() const;
};

//===== Echo =====
class EchoRequest: public Message
{
public:
    EchoRequest(uint8_t moduleAddress, uint8_t data0, uint8_t data1);
};

class EchoResponse: public Message
{
public:
    EchoResponse(QByteArray& encodedBuffer);
    uint8_t data0() const;
    uint8_t data1() const;
};

//===== Set Power Supply Channel State =====
class SetPowerSupplyChannelStateRequest: public Message
{
public:
    SetPowerSupplyChannelStateRequest(uint8_t moduleAddress, ChannelID channel, RelayState state);
};

class SetPowerSupplyChannelStateResponse: public OKErrorMessage
{
public:
    SetPowerSupplyChannelStateResponse(QByteArray& encodedBuffer);
};

//===== Get Power Supply Channel State =====
class GetPowerSupplyChannelStateRequest: public Message
{
public:
    GetPowerSupplyChannelStateRequest(uint8_t moduleAddress, ChannelID channel);
};

class GetPowerSupplyChannelStateResponse: public Message
{
public:
    GetPowerSupplyChannelStateResponse(QByteArray& encodedBuffer);
    ChannelID channel() const;
    RelayState state() const;
};

//===== Get Power Supply Fuse State =====
class GetPowerSupplyFuseStateRequest: public Message
{
public:
    GetPowerSupplyFuseStateRequest(uint8_t moduleAddress, FuseID fuseID);
};

class GetPowerSupplyFuseStateResponse: public Message
{
public:
    GetPowerSupplyFuseStateResponse(QByteArray& encodedBuffer);
    FuseID fuseID() const;
    FuseState fuseState() const;
};

//===== Get Channel Telemetry =====
class GetChannelTelemetryRequest: public Message
{
public:
    GetChannelTelemetryRequest(uint8_t moduleAddress, int channelID);
};

class GetChannelTelemetryResponse: public Message
{
public:
    GetChannelTelemetryResponse(QByteArray& encodedBuffer);
    qreal voltage() const;
};

//===== Set MKO Power Supply Channel State =====
class SetMKOPowerSupplyChannelStateRequest: public Message
{
public:
    SetMKOPowerSupplyChannelStateRequest(uint8_t moduleAddress, MKOChannelID channel, RelayState state);
};

class SetMKOPowerSupplyChannelStateResponse: public OKErrorMessage
{
public:
    SetMKOPowerSupplyChannelStateResponse(QByteArray& encodedBuffer);
};

//===== Get MKO Power Supply Channel State =====
class GetMKOPowerSupplyChannelStateRequest: public Message
{
public:
    GetMKOPowerSupplyChannelStateRequest(uint8_t moduleAddress, MKOChannelID channel);
};

class GetMKOPowerSupplyChannelStateResponse: public Message
{
public:
    GetMKOPowerSupplyChannelStateResponse(QByteArray& encodedBuffer);
    MKOChannelID channel() const;
    RelayState state() const;
};

//===== Get PT-100 Temperature =====
class GetPT100TemperatureRequest: public Message
{
public:
    GetPT100TemperatureRequest(uint8_t moduleAddress, int sensorID);
};

class GetPT100TemperatureResponse: public Message
{
public:
    GetPT100TemperatureResponse(QByteArray& encodedBuffer);
    qreal temperature() const;
    bool hasMeasurementError() const;
};

//===== Get Sensors Count Line 1 =====
class GetSensorsCountLine1Request: public Message
{
public:
    GetSensorsCountLine1Request(uint8_t moduleAddress);
};

class GetSensorsCountLine1Response: public OKErrorMessage
{
public:
    GetSensorsCountLine1Response(QByteArray& encodedBuffer);
    int count() const;
};

//===== Get Sensors Count Line 2 =====
class GetSensorsCountLine2Request: public Message
{
public:
    GetSensorsCountLine2Request(uint8_t moduleAddress);
};

class GetSensorsCountLine2Response: public OKErrorMessage
{
public:
    GetSensorsCountLine2Response(QByteArray& encodedBuffer);
    int count() const;
};

//===== Get Sensor Address Line 1 ===== TODO
//===== Get Sensor Address Line 2 ===== TODO

//===== Reset Line 1 =====
class ResetLine1Request: public Message
{
public:
    ResetLine1Request(uint8_t moduleAddress);
};

class ResetLine1Response: public OKErrorMessage
{
public:
    ResetLine1Response(QByteArray& encodedBuffer);
};

//===== Reset Line 2 =====
class ResetLine2Request: public Message
{
public:
    ResetLine2Request(uint8_t moduleAddress);
};

class ResetLine2Response: public OKErrorMessage
{
public:
    ResetLine2Response(QByteArray& encodedBuffer);
};

//===== Start Temperature Measurement Line 1 =====
class StartMeasurementLine1Request: public Message
{
public:
    StartMeasurementLine1Request(uint8_t moduleAddress);
};

class StartMeasurementLine1Response: public OKErrorMessage
{
public:
    StartMeasurementLine1Response(QByteArray& encodedBuffer);
};

//===== Start Temperature Measurement Line 2 =====
class StartMeasurementLine2Request: public Message
{
public:
    StartMeasurementLine2Request(uint8_t moduleAddress);
};

class StartMeasurementLine2Response: public OKErrorMessage
{
public:
    StartMeasurementLine2Response(QByteArray& encodedBuffer);
};

//===== Get DS1820 Temperature Line 1 =====
class GetDS1820TemperatureLine1Request: public Message
{
public:
    GetDS1820TemperatureLine1Request(uint8_t moduleAddress, int sensorID);
};

class GetDS1820TemperatureLine1Response: public Message
{
public:
    GetDS1820TemperatureLine1Response(QByteArray& encodedBuffer);
    qreal temperature() const;
};

//===== Get DS1820 Temperature Line 2 =====
class GetDS1820TemperatureLine2Request: public Message
{
public:
    GetDS1820TemperatureLine2Request(uint8_t moduleAddress, int sensorID);
};

class GetDS1820TemperatureLine2Response: public Message
{
public:
    GetDS1820TemperatureLine2Response(QByteArray& encodedBuffer);
    qreal temperature() const;
};

// TODO Tech module commands
}

#endif // OKB_CODEC_H
