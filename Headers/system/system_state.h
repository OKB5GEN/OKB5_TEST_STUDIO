#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <QMap>

#include "Headers/logic/variable_controller.h"

class QSerialPort;
class MKO;
class OTD;

class SystemState: public VariableController
{
    Q_OBJECT

public:
    SystemState(QObject* parent);
    ~SystemState();

    void init();

private slots:
    int simpltst1(int x);

    void plot_point();
    void paintvalue();
    void OTDPTdata(double x,double y);
    void OTDtemd(QString data);
    void OTDerror(QString err);
    void OTDtm1(QString temp);
    void OTDtm2(QString temp);
    void OTD_fw(double x);
    void OTD_err_res(int x);
    void OTD_id();
    void status_OTD(QString data);
    void MKO_data(QString data);
    void MKO_cm_data(QString data);
    void MKO_change_ch(int x,int y);
    void statusM();
    void statusRS();
    void statusCAN();

    void on_pushButton_U1_clicked();
    void on_pushButton_U2_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_start_com6_clicked();
    void on_pushButton_start_com5_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_ctm_ch_1_clicked();
    void on_pushButton_ctm_ch_0_clicked();
    void on_pushButton_ctm_ch_2_clicked();
    void on_pushButton_ctm_ch_3_clicked();
    void on_pushButton_ctm_ch_4_clicked();
    void on_pushButton_ctm_ch_5_clicked();
    void on_pushButton_ctm_ch_6_clicked();
    void on_pushButton_ctm_ch_7_clicked();
    void on_pushButton_ctm_ch_8_clicked();
    void on_pushButton_ctm_ch_9_clicked();
    void on_pushButton_ctm_ch_10_clicked();
    void on_pushButton_ctm_ch_11_clicked();
    void on_pushButton_ctm_ch_12_clicked();
    void on_pushButton_ctm_ch_13_clicked();
    void on_pushButton_ctm_ch_14_clicked();
    void on_pushButton_ctm_ch_15_clicked();
    void on_pushButton_check_fuse_1_clicked();
    void on_pushButton_tech_fd_clicked();
    void on_pushButton_tech_hd_clicked();
    void on_tech_set_speed_clicked();
    void on_pushButton_send_tech_clicked();
    void on_tech_clear_out_3_clicked();
    void on_tech_clear_in_3_clicked();
    void on_tech_clear_buf_3_clicked();
    void on_tech_clear_out_4_clicked();
    void on_tech_clear_in_4_clicked();
    void on_tech_clear_buf_4_clicked();
    void on_pushButton_send_tech_2_clicked();
    void on_res_err_stm_clicked();
    void on_res_err_tech_clicked();
    void on_pushButton_res_stm_clicked();
    void on_pushButton_res_tech_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_check_fuse_2_clicked();
    void on_OTDPT_clicked();
    void on_OTD_reset_1_clicked();
    void on_OTD_reset_2_clicked();
    void on_OTD_meas_1_clicked();
    void on_OTD_meas_2_clicked();
    void on_OTD_nd_clicked();
    void OTD_res_st(int x);
    void on_pow_DY_osn_clicked();
    void on_pow_DY_rez_clicked();
    void on_MKO_osn_clicked();
    void on_MKO_rez_clicked();
    void on_MKO_test_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_12_clicked();
    void on_MKO_avt_clicked();
    void on_OTD_avt_2_clicked();
    void on_pushButton_13_clicked();
    void on_pushButton_res_otd_clicked();
    void on_res_err_otd_clicked();

signals:
    void OTD1();
    void OTD_reset1();
    void OTD_reset2();
    void OTD_meas1();
    void OTD_meas2();
    void OTD_tm1();
    void OTD_tm2();
    void OTD_nd();
    void OTD_auto(int x,int y);
    void OTD_sfw();
    void OTD_res();
    void OTD_err();
    void OTD_req();
    void MKO_stop();
    void MKO_DY(int x, int y);
    void MKO_ts(int x,int y, int z);
    void MKO_cm(int x,QString y,int z, int k);
    void MKO_cm_r(int x,int y,int z);
    void MKO_ch(int x);
    void MKO_auto(int x,int y,int adr1, int adr2);

private:
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

    void setPowerValue(uint8_t valueID, double value, double maxValue, QSerialPort * port);
    QSerialPort* createPort(const QString& name);
    QByteArray send(QSerialPort * port, QByteArray data); // returns response

    void setActive(ModuleID id, bool state);
    void resetPort(ModuleID id);

    QSerialPort* getPort(ModuleID id);
    bool isActive(ModuleID id) const;
    uint8_t getAddress(ModuleID id) const;

    QThread *mThreadOTD;
    OTD *mOTD;
    QThread *mThreadMKO;
    MKO *mMKO;

    int m_mko_kits;

    int m_flag_rem1 = 0;
    int m_flag_rem2 = 0;
    int m_flag_con1 = 0;
    int m_flag_con2 = 0;
    int m_k = 0;
    int m_flag_con3 = 0;
    int m_flag_con4 = 0;
    int m_flag_con5 = 0;
    int m_flag_mko_auto = 0;
    int m_flag_otd_auto = 0;
    int m_dat[1000]={0};
    int m_dat1[1000]={0};

    QMap<ModuleID, ModuleInfo> m_modules;
};
#endif // SYSTEM_STATE_H
