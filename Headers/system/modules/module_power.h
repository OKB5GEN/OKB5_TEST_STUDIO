#ifndef MODULE_POWER_H
#define MODULE_POWER_H

#include "Headers/system/com_port_module.h"

class QTimer;

class ModulePower: public COMPortModule
{
    Q_OBJECT

public:
    ModulePower(QObject* parent);
    ~ModulePower();

    bool postInit() override;
    void resetError() override;

    void setUpdatePeriod(int msec, bool startTimer = true);

public slots:
    void startPower();
    void setPowerState(ModuleCommands::PowerState state);
    void setVoltageAndCurrent(qreal voltage, qreal current);
    void setMaxVoltageAndCurrent(qreal voltage, qreal current);

    void voltageAndCurrent();

private slots:
    void update();

signals:
    void changedUI(qreal,qreal);
    void gotUI(qreal, qreal,uint8_t);

private:
    enum ValueID
    {
        MAX_VOLTAGE_VAL = 0x26,
        MAX_CURRENT_VAL = 0x27,
        CUR_VOLTAGE_VAL = 0x32,
        CUR_CURRENT_VAL = 0x33
    };

    void setValue(uint8_t valueID, qreal value, qreal maxValue);
    void getCurVoltageAndCurrent(qreal& voltage, qreal& current, uint8_t& error);

    ModuleCommands::PowerState mState;

    QTimer* mUpdateTimer;
    int mUpdatePeriod;

    double mVoltage;
    double mCurrent;
    uint8_t mError;
};

#endif // MODULE_POWER_H
