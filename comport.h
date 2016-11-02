#ifndef COMPORT_H
#define COMPORT_H

#include <QObject>
#include <QString>
#include <QtSerialPort/QtSerialPort>

class COMPortSender : public QObject
{
    Q_OBJECT

public:
    COMPortSender(QObject *parent = Q_NULLPTR);
    virtual ~COMPortSender();

    void createPorts();

    //TODO check the necessity for public
    void startPower();
    int id_stm();
    int id_tech();
    int stm_on_mko(int y,int x);
    void Reset_error_com6();
    void Reset_error_com5();
    int readerr4I();
    int readerr11I();
    void com6ON();
    void com6OFF();
    void com5ON();
    void com5OFF();
    int readcom5U();
    int readcom5I();
    int readcom6U();
    int readcom6I();
    void setUIcom5(double u);
    void setUIcom6(double u);
    void setoverUIcom5(double u,double ii);
    void setoverUIcom6(double u,double ii);
    QString req_stm();
    QString req_tech();
    int res_err_stm();
    int res_err_tech();
    int res_stm();
    int res_tech();
    int fw_stm();
    int fw_tech();

    int stm_on_com6(int y,int x);
    int stm_on_com5(int y,int x);

    double ctm_data_ch(int ch);
    int ctm_check_fuse(int fuse);

    int tech_send(int com, int x, int y);
    int tech_read(int x);
    QString tech_read_buf(int x,int len);

private:
    QSerialPort* createPort(const QString& name);

    QSerialPort* m_com4 = Q_NULLPTR; // CTM Module port
    QSerialPort* m_com5 = Q_NULLPTR; // Power module port
    QSerialPort* m_com6 = Q_NULLPTR; // Power module port
    QSerialPort* m_com8 = Q_NULLPTR; // Tech module port

    //TODO Refactor
    int flag_rem;
    QByteArray ba;
    double ii1,ii2;
    uint8_t er1,er2;
    int flag_res_tech=1, flag_res_stm=1;

    //QVector<QSerialPort*> m_ports;

    void Remote_ON();
    void Remote_OFF();
};

#endif
