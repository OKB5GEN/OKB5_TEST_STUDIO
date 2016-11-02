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

    COMPortSender(QObject *parent = Q_NULLPTR);
    virtual ~COMPortSender();

    void createPorts();

    void resetError(ModuleID id);
    void setPowerState(ModuleID id, PowerState state);
    void setVoltageAndCurrent(ModuleID id, double voltage);
    void setMaxVoltageAndCurrent(ModuleID id, double voltage, double current);

    //TODO check the necessity for public and method names
    void startPower();
    int id_stm();
    int id_tech();

    int readerr4I();
    int readerr11I();
    int readcom5U();
    int readcom5I();
    int readcom6U();
    int readcom6I();
    QString req_stm();
    QString req_tech();
    int res_err_stm();
    int res_err_tech();
    int res_stm();
    int res_tech();
    int fw_stm();
    int fw_tech();

    int stm_on_mko(int y, int x);
    int stm_on_com6(int y, int x);
    int stm_on_com5(int y, int x);

    double stm_data_ch(int ch);
    int stm_check_fuse(int fuse);

    int tech_send(int com, int x, int y);
    int tech_read(int x);
    QString tech_read_buf(int x, int len);

private:
    QSerialPort* createPort(const QString& name);

    QByteArray send(QSerialPort * port, QByteArray data); // returns response

    QSerialPort* getPort(ModuleID id);


    QMap<ModuleID, QSerialPort*> m_ports;


    //TODO Refactor
    double m_ii1;
    double m_ii2;
    uint8_t m_er1;
    uint8_t m_er2;
    int m_flag_res_tech = 1;
    int m_flag_res_stm = 1;

    void Remote_OFF();
};

#endif
