#ifndef MODULE_STM_H
#define MODULE_STM_H

#include <QVector>

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

    // 1-3 POW_ANT_DRV_CTRL channels (1 and 2 used as main and reserve), 4-6 channels
    void setPowerChannelState(int channel, ModuleCommands::PowerState state); // Подача питания на БУП и ПНА
    void setMKOPowerChannelState(int channel, ModuleCommands::PowerState state); // Подача питания на МКО
    ModuleCommands::PowerState powerChannelState(int channel);
    FuseStates fuseState(int fuseIndex);

public slots:
    void processCustomCommand(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response) override;

private:
    QVector<ModuleCommands::PowerState> mChannelStates;
};

#endif // MODULE_STM_H
