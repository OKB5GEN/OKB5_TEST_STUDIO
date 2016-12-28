#ifndef MODULE_POWER_H
#define MODULE_POWER_H

#include "Headers/system/com_port_module.h"

class ModulePower: public COMPortModule
{
    Q_OBJECT

public:
    ModulePower(QObject* parent);
    ~ModulePower();

private:
};

#endif // MODULE_POWER_H
