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
    void processCustomCommand() override;
    void onApplicationFinish();

protected:
    void onModuleError() override;
    bool processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response) override;
    void createResponse(Transaction& response) override;

signals:
    void powerRelayStateChanged(ModuleCommands::PowerSupplyChannelID channel, int state);
    void powerMKORelayStateChanged(ModuleCommands::MKOPowerSupplyChannelID channel, int state);

private:
};

#endif // MODULE_STM_H
