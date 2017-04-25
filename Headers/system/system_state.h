#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <QMap>
#include <QStringList>

#include "Headers/system/abstract_module.h"
#include "Headers/module_commands.h"

class QSerialPort;

class ModuleMKO;
class ModuleOTD;
class ModuleSTM;
class ModuleTech;
class ModulePower;
class CmdActionModule;

class SystemState: public QObject
{
    Q_OBJECT

public:
    enum ParamID // command parameters
    {
        // explicit commans params (can be changed by the user)
        VOLTAGE               = 0x00000006,
        CURRENT               = 0x00000007,
        TEMPERATURE           = 0x00000008,
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

        // implicit command params (can not be changed by the user)
        SUBADDRESS            = 0x00000017,
        CHANNEL_ID            = 0x00000018,
        POWER_STATE           = 0x00000019,

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

    bool isImplicit(const QString& name) const;
    ParamID paramID(const QString& name) const;
    QString paramName(ParamID param) const;
    QString paramDefaultVarName(ParamID param) const;
    QString paramDefaultDesc(ParamID param) const;

    void sendCommand(CmdActionModule* command);

private slots:
    void processResponse(const Transaction& response);
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
    void sendToSTM(const Transaction& request);
    void sendToOTD(const Transaction& request);
    void sendToTech(const Transaction& request);
    void sendToMKO(const Transaction& request);
    void sendToPowerUnitBUP(const Transaction& request);
    void sendToPowerUnitPNA(const Transaction& request);

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

    QMap<int, QStringList> mInParams[ModuleCommands::MODULES_COUNT];
    QMap<int, QStringList> mOutParams[ModuleCommands::MODULES_COUNT];

    CmdActionModule* mCurCommand;
    QMap<ParamID, QString> mParamNames;
    QMap<ParamID, QString> mDefaultVariables;
    QMap<ParamID, QString> mDefaultDescriptions;

    QMap<ModuleCommands::ModuleID, AbstractModule*> mModules;
};
#endif // SYSTEM_STATE_H
