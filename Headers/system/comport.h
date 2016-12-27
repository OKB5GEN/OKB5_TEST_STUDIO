#ifndef COMPORT_H
#define COMPORT_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QtSerialPort/QtSerialPort>

class COMPortSender : public QObject
{
    Q_OBJECT

public:
    enum ModuleID
    {
        STM,             // STM Module
        TECH,            // Tech Module
        POW_ANT_DRV_CTRL,// Antenna drive control power unit
        POW_ANT_DRV      // Antenna drive power unit
    };

    enum PowerState
    {
        POWER_ON,
        POWER_OFF
    };

    enum InterfaceID
    {
        CAN,
        RS485
    };

    enum CommandID
    {
        GET_MODULE_ADDRESS              = 0x01,
        GET_STATUS_WORD                 = 0x02,
        RESET_ERROR                     = 0x03,
        SOFT_RESET                      = 0x04,
        //RESERVED_0x05 = 0x05,
        GET_SOWFTWARE_VER               = 0x06,
        ECHO                            = 0x07,
        //RESERVED_0x08 = 0x08,
        //RESERVED_0x09 = 0x09,
        //RESERVED_0x0A = 0x0A,
        POWER_CHANNEL_CTRL              = 0x0B, // Can be sent to STM only
        GET_PWR_MODULE_FUSE_STATE       = 0x0C, // Can be sent to STM only
        GET_CHANNEL_TELEMETRY           = 0x0D, // Can be sent to STM only
        SET_MKO_PWR_CHANNEL_STATE       = 0x0E, // Can be sent to STM only
        //MATRIX_CMD_CTRL                 = 0x0F, // Can be sent to MKU only
        SET_PACKET_SIZE_CAN             = 0x10, // Can be sent to TECH only
        ADD_BYTES_CAN                   = 0x11, // Can be sent to TECH only
        SEND_PACKET_CAN                 = 0x12, // Can be sent to TECH only
        CHECK_RECV_DATA_CAN             = 0x13, // Can be sent to TECH only
        RECV_DATA_CAN                   = 0x14, // Can be sent to TECH only
        CLEAN_BUFFER_CAN                = 0x15, // Can be sent to TECH only
        SET_PACKET_SIZE_RS485           = 0x16, // Can be sent to TECH only
        ADD_BYTES_RS485                 = 0x17, // Can be sent to TECH only
        SEND_PACKET_RS485               = 0x18, // Can be sent to TECH only
        CHECK_RECV_DATA_RS485           = 0x19, // Can be sent to TECH only
        RECV_DATA_RS485                 = 0x1A, // Can be sent to TECH only
        CLEAN_BUFFER_RS485              = 0x1B, // Can be sent to TECH only
        GET_TEMPERATURE_PT100           = 0x1C, // Can be sent to OTD only
        GET_DS1820_COUNT_LINE_1         = 0x1D, // Can be sent to OTD only (Psi)
        GET_DS1820_COUNT_LINE_2         = 0x1E, // Can be sent to OTD only (Nu)
        GET_TEMPERATURE_DS1820_LINE_1   = 0x1F, // Can be sent to OTD only (Psi)
        GET_TEMPERATURE_DS1820_LINE_2   = 0x20, // Can be sent to OTD only (Nu)
        GET_POWER_MODULE_STATE          = 0x21, // Can be sent to STM only
        //GET_MKU_MODULE_STATE            = 0x22, // Can be sent to MKU only
        GET_MKO_MODULE_STATE            = 0x23, // Can be sent to STM only
        SET_MODE_RS485                  = 0x24, // Can be sent to TECH only
        SET_SPEED_RS485                 = 0x25, // Can be sent to TECH only
        RESET_LINE_1                    = 0x26, // Can be sent to OTD only (Psi)
        RESET_LINE_2                    = 0x27, // Can be sent to OTD only (Nu)
        START_MEASUREMENT_LINE_1        = 0x28, // Can be sent to OTD only (Psi) 1-2 seconds to perform
        START_MEASUREMENT_LINE_2        = 0x29, // Can be sent to OTD only (Nu) 1-2 seconds to perform
        GET_DS1820_ADDR_LINE_1          = 0x2A, // Can be sent to OTD only (Psi)
        GET_DS1820_ADDR_LINE_2          = 0x2B, // Can be sent to OTD only (Nu)
    };

    COMPortSender(QObject *parent = Q_NULLPTR);
    virtual ~COMPortSender();

    void createPorts();

    int resetError(ModuleID id);
    void setPowerState(ModuleID id, PowerState state);
    void setVoltageAndCurrent(ModuleID id, double voltage);
    void setMaxVoltageAndCurrent(ModuleID id, double voltage, double current);
    int setPowerChannelState(int channel, PowerState state); // 1-3 POW_ANT_DRV_CTRL channels (1 and 2 used as main and reserve), 4-6 channels
    void getCurVoltageAndCurrent(ModuleID id, double& voltage, double& current, uint8_t& error);
    int getSoftwareVersion(ModuleID id);

    int softResetModule(ModuleID id);

    //TODO check the necessity for public and method names
    void startPower();
    int id_stm();
    int id_tech();

    QString req_stm();
    QString req_tech();

    int stm_on_mko(int y, int x);

    double stm_data_ch(int ch);
    int stm_check_fuse(int fuse);

    int tech_send(int com, int x, int y);
    int tech_read(int x);
    QString tech_read_buf(int x, int len);

private:
    enum ValueID // TODO Power unit documentation
    {
        MAX_VOLTAGE_VAL = 0x26,
        MAX_CURRENT_VAL = 0x27,
        CUR_VOLTAGE_VAL = 0x32,
        CUR_CURRENT_VAL = 0x33
    };

    struct ModuleInfo
    {
        QSerialPort * port;
        bool state; // true/false - module active/inactive
        uint8_t address; // default module address
    };

    void setPowerValue(uint8_t valueID, double value, double maxValue, QSerialPort * port);
    QSerialPort* createPort(const QString& name);
    QByteArray send(QSerialPort * port, QByteArray data); // returns response

    void setActive(ModuleID id, bool state);
    void resetPort(ModuleID id);

    QSerialPort* getPort(ModuleID id);
    bool isActive(ModuleID id) const;
    uint8_t getAddress(ModuleID id) const;

    QMap<ModuleID, ModuleInfo> m_modules;
};

#endif
