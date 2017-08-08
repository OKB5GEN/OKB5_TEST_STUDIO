#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <QSet>

#include "Headers/system/abstract_module.h"
#include "Headers/module_commands.h"

class QTimer;

class ModuleMKO;
class ModuleOTD;
class ModuleDriveSimulator;
class ModuleSTM;
class ModuleTech;
class ModulePower;
class CmdActionModule;

class SystemState: public QObject
{
    Q_OBJECT

public:
    struct Param
    {
        QString name; // localized name (TODO on locale change)
        QString variable; // default/generated name
        QString description; // default/generated description
    };

    struct Command
    {
        QSet<uint32_t> inputParams;
        QSet<uint32_t> outputParams;
    };

    enum ParamID // command parameters
    {
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
        MODULE_ADDRESS,
        MODULE_READY,
        MODULE_AFTER_RESET,
        MODULE_HAS_ERRORS,
        SUBADDRESS,
        CHANNEL_ID,
        POWER_STATE,
        STATUS_PHYSICAL, // 0 - inactive, 1 - active
        STATUS_LOGICAL, // 0 - inactive, 1 - active
        ECHO_DATA_1,
        ECHO_DATA_2,
        FUSE_ID,
        FUSE_STATE,
        SENSOR_NUMBER,
        SENSORS_COUNT,

        UNDEFINED
    };

    Q_ENUM(ParamID)

    SystemState(QObject* parent);
    ~SystemState();

    static bool isSetter(ModuleCommands::CommandID command);

    void onApplicationStart();
    void onApplicationFinish();

    void sendCommand(CmdActionModule* command);
    void stop();

    QSet<uint32_t> inputParams(int module, int command) const;
    QSet<uint32_t> outputParams(int module, int command) const;

    Param paramData(ParamID param) const;

    bool isImplicit(ParamID param) const;

private slots:
    void processResponse(const Transaction& response);
    void onResponseTimeout();

signals:
    void commandFinished(bool success);

    void sendToSTM(const Transaction& request);
    void sendToOTD(const Transaction& request);
    void sendToDS(const Transaction& request);
    void sendToTech(const Transaction& request);
    void sendToMKO(const Transaction& request);
    void sendToPowerUnitBUP(const Transaction& request);
    void sendToPowerUnitPNA(const Transaction& request);

private:
    void createPowerUnitCommandsParams();
    void createMKOCommandsParams();
    void createOTDCommandsParams();
    void createDSCommandsParams();
    void createTechCommandsParams();
    void createSTMCommandsParams();

    bool processLocalCommand(Transaction& transaction);

    AbstractModule* moduleByID(ModuleCommands::ModuleID moduleID) const;

    void updateParams();

    void addParam(uint32_t id, const QString& name, const QString& variable, const QString& description);
    void addCommand(uint32_t id, std::initializer_list<uint32_t> input, std::initializer_list<uint32_t> output, QMap<uint32_t, Command>* commands);

    ModuleMKO* mMKO;
    ModuleOTD* mOTD;
    ModuleDriveSimulator* mDS;
    ModuleSTM* mSTM;
    ModuleTech* mTech;
    ModulePower* mPowerBUP;
    ModulePower* mPowerPNA;

    QMap<uint32_t, Command> mCommands[ModuleCommands::MODULES_COUNT];
    QMap<uint32_t, Param> mParams;

    CmdActionModule* mCurCommand;

    QTimer* mProtectionTimer; //TODO parallel process (сейчас SystemState не может обрабатывать команды параллельно!)
};
#endif // SYSTEM_STATE_H
