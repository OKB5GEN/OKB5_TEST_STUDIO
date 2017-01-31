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
    enum ParamID // command parameters
    {
        // common command params
        MODULE_ID           = 0x00000000,
        COMMAND_ID          = 0x00000001,
        INPUT_PARAMS_COUNT  = 0x00000002,
        OUTPUT_PARAMS_COUNT = 0x00000003,
        ERROR_CODE          = 0x00000004,

        // param types
        VOLTAGE             = 0x00000005,
        CURRENT             = 0x00000006,

        // custom command params
        INPUT_PARAM_BASE    = 0x00001000, // all input params will have code "in base + i"
        OUTPUT_PARAM_BASE   = 0x10000000, // all output params will have code "out base + i"

        UNDEFINED           = 0xffffffff
    };

    Q_ENUM(ParamID)

    SystemState(QObject* parent);
    ~SystemState();

    void init();

    QString paramName(int module, int command, int param, bool isInputParam) const;
    int paramsCount(int module, int command, bool isInputParam) const;

    QString paramName(ParamID param) const;
    ParamID paramID(const QString& name) const;

    void sendCommand(CmdActionModule* command);

private slots:

    //TODO refactor/remove
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
    //<<<

    // new slots
    void processResponse(const QMap<uint32_t, QVariant>& response);

signals:
    void commandFinished(bool success);

    // TODO refactor >>>
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
    // <<<

    // new signals
    void sendToSTM(const QMap<uint32_t, QVariant>& request);
    void sendToOTD(const QMap<uint32_t, QVariant>& request);
    void sendToTech(const QMap<uint32_t, QVariant>& request);
    void sendToMKO(const QMap<uint32_t, QVariant>& request);
    void sendToPowerUnitBUP(const QMap<uint32_t, QVariant>& request);
    void sendToPowerUnitPNA(const QMap<uint32_t, QVariant>& request);

private:
    void setupParams();
    void onExecutionFinished(uint32_t error);

    ModuleMKO* moduleMKO() const;
    ModuleOTD* moduleOTD() const;
    ModuleSTM* moduleSTM() const;
    ModuleTech* moduleTech() const;
    ModulePower* modulePowerBUP() const;
    ModulePower* modulePowerPNA() const;

    bool sendPowerUnitCommand(CmdActionModule* command);
    bool sendOTDCommand(CmdActionModule* command);
    bool sendSTMCommand(CmdActionModule* command);
    bool sendMKOCommand(CmdActionModule* command);
    bool sendTechCommand(CmdActionModule* command);

    bool createMKO();
    bool createOTD();
    bool createSTM();
    bool createTech();
    bool createPowerBUP();
    bool createPowerPNA();

    ModuleMKO* mMKO;
    ModuleOTD* mOTD;
    ModuleSTM* mSTM;
    ModuleTech* mTech;
    ModulePower* mPowerBUP;
    ModulePower* mPowerPNA;

    int m_mko_kits; //TODO remove/move to MKO

    QThread* mThreadMKO;
    QThread* mThreadOTD;

    QMap<int, QStringList> mInParams[ModuleCommands::MODULES_COUNT];
    QMap<int, QStringList> mOutParams[ModuleCommands::MODULES_COUNT];

    CmdActionModule* mCurCommand;

    QMap<ParamID, QString> mParamNames;
};
#endif // SYSTEM_STATE_H
