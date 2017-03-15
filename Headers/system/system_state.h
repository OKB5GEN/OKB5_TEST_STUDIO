#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <QMap>
#include <QStringList>

#include "Headers/system/abstract_module.h"
#include "Headers/logic/variable_controller.h" //TODO what for?
#include "Headers/module_commands.h"

class QSerialPort;

class ModuleMKO;
class ModuleOTD;
class ModuleSTM;
class ModuleTech;
class ModulePower;
class CmdActionModule;

class SystemState: public QObject//: public VariableController //TODO possibly it's better to inherit from QObject
{
    Q_OBJECT

public:
    enum ParamID // command parameters
    {
        // common command params
        MODULE_ID             = 0x00000000,
        COMMAND_ID            = 0x00000001,
        INPUT_PARAMS_COUNT    = 0x00000002,
        OUTPUT_PARAMS_COUNT   = 0x00000003,
        IMPLICIT_PARAMS_COUNT = 0x00000004,
        ERROR_CODE            = 0x00000005,

        // param types
        VOLTAGE               = 0x00000006,
        CURRENT               = 0x00000007,
        TEMPERATURE           = 0x00000008,

        //MKO param types
        MODE_PSY              = 0x00000009,
        STEPS_PSY             = 0x0000000A,
        VELOCITY_PSY          = 0x0000000B,
        CURRENT_PSY           = 0x0000000C,
        ANGLE_PSY             = 0x0000000D,
        MODE_NU               = 0x0000000E,
        STEPS_NU              = 0x0000000F,
        VELOCITY_NU           = 0x00000010,
        CURRENT_NU            = 0x00000011,
        ANGLE_NU              = 0x00000012,
        SENSOR_FLAG           = 0x00000013,
        MODE                  = 0x00000014,
        STEPS                 = 0x00000015,
        VELOCITY              = 0x00000016,

        // custom command params
        INPUT_PARAM_BASE      = 0x00001000, // all input params will have code "in base + i"
        OUTPUT_PARAM_BASE     = 0x00100000, // all output params will have code "out base + i"
        IMPLICIT_PARAM_BASE   = 0x10000000, // all implicit input params will have code "implicit base + i"

        UNDEFINED             = 0xffffffff
    };

    Q_ENUM(ParamID)

    SystemState(QObject* parent);
    ~SystemState();

    void onApplicationStart();

    void setDefaultState();
    void onCyclogramStart(); //TODO remove

    QString paramName(int module, int command, int param, bool isInputParam) const;
    int paramsCount(int module, int command, bool isInputParam) const;

    ParamID paramID(const QString& name) const;
    QString paramName(ParamID param) const;
    QString paramDefaultVarName(ParamID param) const;
    QString paramDefaultDesc(ParamID param) const;

    void sendCommand(CmdActionModule* command);

private slots:

    //TODO refactor/remove
    int simpltst1(int x);

    void MKO_data(QString data);
    void MKO_cm_data(QString data);

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
    void on_MKO_osn_clicked();
    void on_MKO_rez_clicked();
    void on_MKO_test_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_12_clicked();
    void on_MKO_avt_clicked();
    //<<<

    // new slots
    void processResponse(const QMap<uint32_t, QVariant>& response);
    void onModuleStateChanged(ModuleCommands::ModuleID moduleID, AbstractModule::ModuleState from, AbstractModule::ModuleState to);

signals:
    void commandFinished(bool success);

    // TODO refactor >>>
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
    void setupCommandsParams();

    void onExecutionFinished(uint32_t error);

    bool sendPowerUnitCommand(CmdActionModule* command);
    bool sendOTDCommand(CmdActionModule* command);
    bool sendSTMCommand(CmdActionModule* command);
    bool sendMKOCommand(CmdActionModule* command);
    bool sendTechCommand(CmdActionModule* command);

    void createPowerUnitCommandsParams();
    void createOTDCommandsParams();

    ModuleMKO* mMKO;
    ModuleOTD* mOTD;
    ModuleSTM* mSTM;
    ModuleTech* mTech;
    ModulePower* mPowerBUP;
    ModulePower* mPowerPNA;

    int mEnablesMKOKits; //TODO remove/move to MKO

    QMap<int, QStringList> mInParams[ModuleCommands::MODULES_COUNT];
    QMap<int, QStringList> mOutParams[ModuleCommands::MODULES_COUNT];

    CmdActionModule* mCurCommand;
    QMap<ParamID, QString> mParamNames;
    QMap<ParamID, QString> mDefaultVariables;
    QMap<ParamID, QString> mDefaultDescriptions;

    QMap<ModuleCommands::ModuleID, AbstractModule*> mModules;
};
#endif // SYSTEM_STATE_H
