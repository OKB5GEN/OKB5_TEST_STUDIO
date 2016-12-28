#ifndef MODULE_TECH_H
#define MODULE_TECH_H

#include "Headers/system/com_port_module.h"

class ModuleTech: public COMPortModule
{
    Q_OBJECT

public:
    ModuleTech(QObject* parent);
    ~ModuleTech();

private:
};

#endif // MODULE_TECH_H
