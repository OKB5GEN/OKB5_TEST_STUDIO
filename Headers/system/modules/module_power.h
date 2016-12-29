#ifndef MODULE_POWER_H
#define MODULE_POWER_H

#include "Headers/system/com_port_module.h"

class ModulePower: public COMPortModule
{
    Q_OBJECT

public:
    ModulePower(QObject* parent);
    ~ModulePower();

    bool postInit() override;
    void resetError() override;

    void startPower();
    void setPowerState(ModuleCommands::PowerState state);
    void setVoltageAndCurrent(double voltage);
    void setMaxVoltageAndCurrent(double voltage, double current);
    void getCurVoltageAndCurrent(double& voltage, double& current, uint8_t& error);

private:
    enum ValueID
    {
        MAX_VOLTAGE_VAL = 0x26,
        MAX_CURRENT_VAL = 0x27,
        CUR_VOLTAGE_VAL = 0x32,
        CUR_CURRENT_VAL = 0x33
    };

    void setPowerValue(uint8_t valueID, double value, double maxValue);

    ModuleCommands::PowerState mState;
};

#endif // MODULE_POWER_H
