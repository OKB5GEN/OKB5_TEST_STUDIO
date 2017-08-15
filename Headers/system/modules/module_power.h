#ifndef MODULE_POWER_H
#define MODULE_POWER_H

#include "Headers/system/com_port_module.h"

class QTimer;

class ModulePower: public COMPortModule
{
    Q_OBJECT

public:
    ModulePower(QObject* parent);
    ~ModulePower();

public slots:
    void processCommand(const Transaction& params) override;
    void onApplicationFinish();

protected:
    bool processResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response) override;

    void onTransmissionError(uint32_t operationID) override;
    void onTransmissionComplete() override;

private:
    enum ObjectID
    {
        DEVICE_TYPE           =  0, // Read-only
        DEVICE_SERIAL_NUMBER  =  1, // Read-only
        NOMINAL_VOLTAGE       =  2, // Read-only
        NOMINAL_CURRENT       =  3, // Read-only
        NOMINAL_POWER         =  4, // Read-only
        DEVICE_ARTICLE_NUMBER =  6, // Read-only
        MANUFACTURER          =  8, // Read-only
        SOWFTWARE_VERSION     =  9, // Read-only
        DEVICE_CLASS          = 19, // Read-only
        OVP_THRESHOLD         = 38, // Read-Write
        OCP_THRESHOLD         = 39, // Read-Write
        SET_VALUE_U           = 50, // Read-Write
        SET_VALUE_I           = 51, // Read-Write
        POWER_SUPPLY_CONTROL  = 54, // Read-only
        DEVICE_STATUS_ACTUAL  = 71, // Read-only. "Actual" device state (at current time moment)
        DEVICE_STATUS_MOM_SET = 72, // Read-only. "Future" device state (need to be set)
    };

    // DEVICE_STATUS_ACTUAL and DEVICE_STATUS_MOM_SET will be equal after a few seconds

    enum PowerSupplyCommandID
    {
        SWITCH_POWER_OUTPUT_ON  = 0x0101,
        SWITCH_POWER_OUTPUT_OFF = 0x0100,
        ACKNOWLEDGE_ALARMS      = 0x0a0a,
        SWITCH_TO_REMOTE_CTRL   = 0x1010,
        SWITCH_TO_MANUAL_CTRL   = 0x1000,
        TRACKING_ON             = 0xF0F0,
        TRACKING_OFF            = 0xF0E0,
    };

    enum DeviceNode
    {
        SINGLE_MODEL         = 0x00, // we use single-models PS2000 device series
        DC_OUTPUT_1          = 0x00, // not used (used in triple-model PS2000 device series)
        DC_OUTPUT_2          = 0x01, // not used (used in triple-model PS2000 device series)
    };

    enum DeviceClass
    {
        SINGLE = 0x0010,
        TRIPLE = 0x0018
    };

    // start delimiter params
    enum Direction
    {
        FROM_DEVICE = 0x00,
        TO_DEVICE   = 0x10
    };

    enum CastType
    {
        SEND    = 0x20,
        RECEIVE = 0x00
    };

    enum TransmissionType
    {
        RESERVED        = 0x00,
        QUERY_DATA      = 0x40,
        ANSWER_TO_QUERY = 0x80,
        SEND_DATA       = 0xc0
    };

    enum PowerState
    {
        POWER_ON,
        POWER_OFF
    };

    void getCurVoltageAndCurrent();
    void setVoltageAndCurrent(qreal voltage);

    // power units command
    void sendPowerSupplyControlCommand(PowerSupplyCommandID command);
    void setObjectValue(ObjectID objectID, qreal actualValue, qreal nominalValue);
    void getNominalValue(ObjectID objectID);
    void getObjectValue(ObjectID objectID);
    void getDeviceClass();

    // encoding/decoding
    static uint8_t encodeStartDelimiter(TransmissionType trType, uint8_t dataSize);
    static void addCheckSum(QByteArray& data);

    PowerState mState;

    qreal mVoltageThreshold;
    qreal mCurrentThreshold;
    qreal mVoltage;
    qreal mCurrent;
    qreal mNominalVoltage;
    qreal mNominalCurrent;
    qreal mNominalPower;

    uint16_t mDeviceClass; //TODO not used, we use SIMPLE module version
    uint8_t mError;
};

#endif // MODULE_POWER_H
