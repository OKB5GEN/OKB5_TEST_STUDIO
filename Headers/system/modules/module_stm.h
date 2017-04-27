#ifndef MODULE_STM_H
#define MODULE_STM_H

#include <QMap>

#include "Headers/system/okb_module.h"

class ModuleSTM: public ModuleOKB
{
    Q_OBJECT

public:
    enum FuseStates
    {
        OK      = 0,
        FAULTY  = 1,
        ERROR   = 2
    };

    Q_ENUM(FuseStates)

    ModuleSTM(QObject* parent);
    ~ModuleSTM();

public slots:
    void processCustomCommand(const Transaction& request, Transaction& response) override;
    void onApplicationFinish();

protected:
    void onModuleError() override;
    bool processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response) override;
    void createResponse(Transaction& response) override;

private:
    void setPowerChannelState(ModuleCommands::PowerSupplyChannelID channel, ModuleCommands::PowerState moduleState); // Подача питания на БУП и ПНА
    void setMKOPowerChannelState(ModuleCommands::MKOPowerSupplyChannelID channel, ModuleCommands::PowerState moduleState); // Подача питания на МКО
    ModuleCommands::PowerState getPowerChannelState(ModuleCommands::PowerSupplyChannelID channel);
    ModuleCommands::PowerState getMKOPowerChannelState(ModuleCommands::MKOPowerSupplyChannelID channel);
    void getChannelTelemetry(int channel);
    FuseStates fuseState(int fuseIndex);

    QMap<ModuleCommands::PowerSupplyChannelID, ModuleCommands::PowerState> mPowerSupplyRelayStates;
    QMap<ModuleCommands::MKOPowerSupplyChannelID, ModuleCommands::PowerState> mMKOPowerSupplyRelayStates;

    int mRequestedChannelTelemetry;
};

#endif // MODULE_STM_H
