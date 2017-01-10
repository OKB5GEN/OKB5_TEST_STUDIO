#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <QMap>
#include <QStringList>

#include "Headers/logic/variable_controller.h"
#include "Headers/module_commands.h"

class QSerialPort;

class ModuleMKO;
class ModuleOTD;
class ModuleSTM;
class ModuleTech;
class ModulePower;
class CmdActionModule;

class SystemState: public VariableController
{
    Q_OBJECT

public:
    SystemState(QObject* parent);
    ~SystemState();

    void init();

    ModuleMKO* moduleMKO() const;
    ModuleOTD* moduleOTD() const;
    ModuleSTM* moduleSTM() const;
    ModuleTech* moduleTech() const;
    ModulePower* modulePowerBUP() const;
    ModulePower* modulePowerPNA() const;

    QString paramName(int module, int command, int param, bool isInputParam) const;
    int paramsCount(int module, int command, bool isInputParam) const;

    void sendCommand(CmdActionModule* command, const char* onFinish);

private slots:
    int simpltst1(int x);

    void OTDtemd(QString data);
    void OTDerror(QString err);
    void OTDtm1(QString temp);
    void OTDtm2(QString temp);
    void status_OTD(QString data);

    void MKO_data(QString data);
    void MKO_cm_data(QString data);
    void MKO_change_ch(int x,int y);

    void checkModulesStatus();

    void on_pushButton_tech_fd_clicked();
    void on_pushButton_tech_hd_clicked();
    void on_tech_set_speed_clicked();
    void on_tech_clear_out_3_clicked();
    void on_tech_clear_in_3_clicked();
    void on_tech_clear_buf_3_clicked();
    void on_tech_clear_out_4_clicked();
    void on_tech_clear_in_4_clicked();
    void on_tech_clear_buf_4_clicked();
    void on_OTD_nd_clicked();
    void on_pow_DY_osn_clicked();
    void on_pow_DY_rez_clicked();
    void on_MKO_osn_clicked();
    void on_MKO_rez_clicked();
    void on_MKO_test_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_12_clicked();
    void on_MKO_avt_clicked();
    void on_OTD_avt_2_clicked();

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

    void commandFinished(bool success);

private:
    void setupParams();

    ModuleMKO* mMKO;
    ModuleOTD* mOTD;
    ModuleSTM* mSTM;
    ModuleTech* mTech;
    ModulePower* mPowerBUP;
    ModulePower* mPowerPNA;

    int m_mko_kits;

    QThread* mThreadMKO;
    QThread* mThreadOTD;

    //int m_flag_rem1 = 0;
    //int m_flag_rem2 = 0;
    //int m_k = 0;
    //int m_flag_mko_auto = 0;
    //int m_flag_otd_auto = 0;
    //int m_dat[1000]={0};
    //int m_dat1[1000]={0};
    QMap<int, QStringList> mInParams[ModuleCommands::MODULES_COUNT];
    QMap<int, QStringList> mOutParams[ModuleCommands::MODULES_COUNT];
};
#endif // SYSTEM_STATE_H
