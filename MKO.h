#ifndef MKO_H
#define MKO_H
#include "minwindef.h"

#include <QObject>
#include <QString>

class QTimer;

class MKO : public QObject
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

    MKO(QString name, QObject * parent);

public slots:
    void startMKO();
    void startMKO1();
    void stopMKO();
    void stopMKO1();
    QString OCcontrol(WORD oc);
    void pow_DY(int kit, int adr);
    void MKO_start_test(int kits, int adr1, int adr2);
    void MKO_tr_cm(int kits, QString cm, int adr1, int adr2);
    void MKO_rc_cm(int kits, int adr1, int adr2);
    void MKO_chan(int kits);
    void MKO_avt(int x, int y, int adr1, int adr2);
    void MKO_timer();

signals:
    void test_MKO(int x);
    void start_MKO(QString x);
    void data_MKO(QString x);
    void MKO_CTM(int x, int y);

private:
    void send(uint16_t* buf, uint16_t len, QString& dat, QString& error);
    void send(uint16_t* buf, uint16_t len);
    QString receive(uint16_t* buf, uint16_t len);
    QString name;
    QTimer * m_timer;

    uint8_t m_addr; // variable module address
    uint8_t m_subAddr; // constant module subaddress

    KitID m_enabledKits = NO_KIT;
};

#endif
