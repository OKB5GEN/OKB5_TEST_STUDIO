#ifndef MODULE_MKO_H
#define MODULE_MKO_H

#include <QObject>

class QTimer;

class ModuleMKO: public QObject
{
    Q_OBJECT

public:

    enum KitID
    {
        NO_KIT      = 0x00,
        MAIN_KIT    = 0x01,
        RESERVE_KIT = 0x02,
        ALL_KITS = MAIN_KIT | RESERVE_KIT
    };

    ModuleMKO(QObject* parent);
    ~ModuleMKO();

public slots:
/*    void startMKO();
    void startMKO1();
    void stopMKO();
    void stopMKO1();
    QString OCcontrol(uint16_t oc);
    void pow_DY(int kit, int adr);
    void MKO_start_test(int kits, int adr1, int adr2);
    void MKO_tr_cm(int kits, QString cm, int adr1, int adr2);
    void MKO_rc_cm(int kits, int adr1, int adr2);
    void MKO_chan(int kits);
    void MKO_avt(int x, int y, int adr1, int adr2);
    void MKO_timer();*/

    void startMKO();
    void startMKO1();
    void stopMKO();
    void stopMKO1();
    QString OCcontrol(uint16_t oc);
    void pow_DY(int x, int y);
    void MKO_start_test(int x, int adr1, int adr2);
    void tx_mes(uint16_t* sendBuffer, uint16_t sendCount, uint16_t* receiveBuffer, uint16_t receiveCount);
    void rx_mes(uint16_t* receiveBuffer, uint16_t receiveCount);
    void MKO_tr_cm(int x,QString cm, int adr1, int adr2);
    void MKO_rc_cm(int x, int adr1, int adr2);
    void MKO_chan(int x);
    void MKO_avt(int x,int y,int adr1, int adr2);
    void MKO_timer();

signals:
    void test_MKO(int x);
    void start_MKO(QString x);
    void data_MKO(QString x);
    void MKO_CTM(int x, int y);

private:
    uint8_t mAddr; // variable module address
    uint8_t mSubAddr; // constant module subaddress

    /*
    void send(uint16_t* buf, uint16_t len, QString& dat, QString& error);
    void send(uint16_t* buf, uint16_t len);
    QString receive(uint16_t* buf, uint16_t len);
    QTimer * m_timer;



    KitID m_enabledKits = NO_KIT;
    */
};

#endif // MODULE_MKO_H
