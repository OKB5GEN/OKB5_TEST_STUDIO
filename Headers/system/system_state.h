#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <QMap>
#include <QStringList>

#include "Headers/system/abstract_module.h"
#include "Headers/module_commands.h"

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
        VOLTAGE,
        CURRENT,
        POWER,
        TEMPERATURE,
        MODE_PSY,
        STEPS_PSY,
        VELOCITY_PSY,
        CURRENT_PSY,
        ANGLE_PSY,
        MODE_NU,
        STEPS_NU,
        VELOCITY_NU,
        CURRENT_NU,
        ANGLE_NU,
        SENSOR_FLAG,
        DRIVE_MODE,
        STEPS,
        VELOCITY,
        DEVICE_CLASS,
        RELAY_STATE, // 0 - off, 1 - on

        // implicit command params (can not be changed by the user)
        SUBADDRESS,
        CHANNEL_ID,
        POWER_STATE,

        STATUS_PHYSICAL, // 0 - inactive, 1 - active
        STATUS_LOGICAL, // 0 - inactive, 1 - active

        UNDEFINED
    };

    Q_ENUM(ParamID)

    SystemState(QObject* parent);
    ~SystemState();

    void onApplicationStart();
    void onApplicationFinish();

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

signals:
    void commandFinished(bool success);

    void sendToSTM(const Transaction& request);
    void sendToOTD(const Transaction& request);
    void sendToTech(const Transaction& request);
    void sendToMKO(const Transaction& request);
    void sendToPowerUnitBUP(const Transaction& request);
    void sendToPowerUnitPNA(const Transaction& request);

private:
    void createPowerUnitCommandsParams();
    void createMKOCommandsParams();
    void createOTDCommandsParams();
    void createTechCommandsParams();
    void createSTMCommandsParams();

    bool processLocalCommand(Transaction& transaction);

    AbstractModule* moduleByID(ModuleCommands::ModuleID moduleID) const;

    ModuleMKO* mMKO;
    ModuleOTD* mOTD;
    ModuleSTM* mSTM;
    ModuleTech* mTech;
    ModulePower* mPowerBUP;
    ModulePower* mPowerPNA;

    QMap<int, QStringList> mInParams[ModuleCommands::MODULES_COUNT];
    QMap<int, QStringList> mOutParams[ModuleCommands::MODULES_COUNT];

    CmdActionModule* mCurCommand;
    //TODO unite strings and data to command params. Commands move to config
    QMap<ParamID, QString> mParamNames;
    QMap<ParamID, QString> mDefaultVariables;
    QMap<ParamID, QString> mDefaultDescriptions;
};
#endif // SYSTEM_STATE_H
