#ifndef MODULE_POWER_BUP_H
#define MODULE_POWER_BUP_H

#include "Headers/system/module.h"

class ModulePowerBUP: public Module
{
    Q_OBJECT

public:
    ModulePowerBUP(QObject* parent);
    ~ModulePowerBUP();

private:
};

#endif // MODULE_POWER_BUP_H
