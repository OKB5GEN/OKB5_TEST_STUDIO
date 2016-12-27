#ifndef OTD_H
#define OTD_H

#include <QObject>
#include <QString>

class QSerialPort;
class QTimer;

class OTD:public QObject
{
    Q_OBJECT

public:
    OTD(QString name, QObject* parent);

public slots:
    void doWork();
    void COMCloseOTD();
    void COMConnectorOTD();
    void OTDPT();
    void OTDtemper();
    void OTDres1();
    void OTDres2();
    void OTDmeas1();
    void OTDmeas2();
    void OTDtm1();
    void OTDtm2();
    void OTD_avt(int,int);
    void OTD_timer();
    void OTD_fw();
    void res_OTD();
    void err_res_OTD();
    void OTD_id();
    void OTD_req();

signals:
    void start_OTD();
    void start_OTDPT(double x, double y);
    void temp_OTD(QString data);
    void err_OTD(QString err);
    void tm_OTD1 (QString temp);
    void tm_OTD2 (QString temp);
    void OTD_vfw(double x);
    void OTD_res(int x);
    void OTD_err_res(int x);
    void OTD_id1();
    void OTD_reqr (QString res);

private:
    QString name;

    QTimer * m_timer;

    QByteArray send(QByteArray data, double readTimeout, double delayBeforeRecv = 0);
    bool m_isActive = false;

    int m_sensorsCntAxis1 = 0;
    int m_sensorsCntAxis2 = 0;

    QSerialPort * m_port = Q_NULLPTR;
};

#endif