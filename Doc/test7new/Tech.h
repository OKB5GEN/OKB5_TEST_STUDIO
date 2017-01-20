#ifndef Tech_H
#define Tech_H

#include <QObject>
#include <QString>

class Tech:public QObject
{
    Q_OBJECT
public:
    Tech(QString name);



public slots:
    void COMCloseTech();
    void COMConnectorTech();
    void COMConnectorRM_2();
    void COMConnectorSKVT();
    void Tech_avt(int,int);
    void Tech_ssi(int);
    void Tech_log (int x, QString S3);
    void Tech_timer();
    QString req_tech();
    int tech_send(int com, int x, int y);
    int tech_read(int x);
    QString tech_read_buf(int x,int len);
    int id_tech();
    int res_err_tech();
    int res_tech();
    int fw_tech();
    int echo_tech();
    void SKVT_RLM();
    void RM_RLM();
    void RLM_RLM();


signals:
    void tech_SSI_value(double x, double y, double skvt_V, double skvt_H);
    void tech_err(double);
    void tech_buf(QString);
    void tech_buf1(QString);



private:
    QString name;
};

#endif
