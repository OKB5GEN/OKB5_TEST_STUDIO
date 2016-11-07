#ifndef MKO_H
#define MKO_H
#include "minwindef.h"

#include <QObject>
#include <QString>

class QTimer;

class MKO:public QObject
{
    Q_OBJECT

public:
    MKO(QString name, QObject * parent);

public slots:
    void startMKO();
    void startMKO1();
    void stopMKO();
    void stopMKO1();
    QString OCcontrol(WORD oc);
    void pow_DY(int x, int y);
    void MKO_start_test(int x, int adr1, int adr2);
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
    void send(uint16_t* buf, uint16_t len, QString& dat, QString& error);
    void send(uint16_t* buf, uint16_t len);
    void receive(uint16_t* buf, uint16_t len, uint16_t* buf1, uint16_t len1, QString& error);
    QString name;
    QTimer * m_timer;

    uint8_t m_addr;
    uint8_t m_subAddr;
};

#endif
