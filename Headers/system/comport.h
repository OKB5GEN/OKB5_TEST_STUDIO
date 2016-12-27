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
