#ifndef MODULE_POWER_H
#define MODULE_POWER_H

#include "Headers/system/com_port_module.h"

#include <QMap>
#include <QVariant>

class QTimer;

class ModulePower: public COMPortModule
{
    Q_OBJECT

public:
    ModulePower(QObject* parent);
    ~ModulePower();

    void initializeCustom() override;
    void resetError() override;

public slots:
    void restart();

    void processCommand(const QMap<uint32_t, QVariant>& params) override;
    void onApplicationFinish() override;

protected:
    void processResponse(const QByteArray& response) override;

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

    enum Operation
    {
        GET_DEVICE_CLASS,
        GET_NOMINAL_CURRENT,
        GET_NOMINAL_VOLTAGE,
        GET_NOMINAL_POWER,
        GET_OVP_THRESHOLD,
        GET_OCP_THRESHOLD,
        GET_CUR_VOLTAGE_AND_CURRENT,
        SET_OVP_THRESHOLD,
        SET_OCP_THRESHOLD,
        SET_SET_VALUE_U,
        SET_SET_VALUE_I,
        PSC_SWITCH_POWER_OUTPUT_ON,
        PSC_SWITCH_POWER_OUTPUT_OFF,
        PSC_ACKNOWLEDGE_ALARMS,
        PSC_SWITCH_TO_REMOTE_CTRL,
        PSC_SWITCH_TO_MANUAL_CTRL,
        PSC_TRACKING_ON,
        PSC_TRACKING_OFF,

        UNKNOWN_OPERATION
    };

    template<typename T>
    static T limitValue(const T& value, const T& nominal, const T& threshold)
    {
        return qMin(qMin(value, nominal), qMin(value, threshold));
    }

    void getCurVoltageAndCurrent();

    //void setMaxVoltageAndCurrent(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response); // TODO not available to user API, possibly must be
    void getVoltageAndCurrent(const QMap<uint32_t, QVariant>& request);
    void setVoltageAndCurrent(const QMap<uint32_t, QVariant>& request);
    void setCurVoltage(qreal voltage);
    void setPowerState(const QMap<uint32_t, QVariant>& request);

    // power units command
    void sendPowerSupplyControlCommand(PowerSupplyCommandID command);
    void setObjectValue(ObjectID objectID, qreal actualValue, qreal nominalValue);
    void getNominalValue(ObjectID objectID);
    void getObjectValue(ObjectID objectID);
    void getDeviceClass();

    // encoding/decoding
    static uint8_t encodeStartDelimiter(TransmissionType trType, uint8_t dataSize);
    static void addCheckSum(QByteArray& data);

    ModuleCommands::PowerState mState;

    qreal mVoltageThreshold;
    qreal mCurrentThreshold;
    qreal mVoltage;
    qreal mCurrent;
    qreal mNominalVoltage;
    qreal mNominalCurrent;
    qreal mNominalPower;

    uint16_t mDeviceClass; //TODO not used, we use SIMPLE module version
    uint8_t mError;

    bool mInitializationFinished;

    QMap<uint32_t, QVariant> mCurrentResponse;
};

#endif // MODULE_POWER_H
