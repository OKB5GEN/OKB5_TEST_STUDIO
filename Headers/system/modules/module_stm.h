#ifndef MODULE_STM_H
#define MODULE_STM_H

#include "Headers/system/com_port_module.h"

class ModuleSTM: public COMPortModule
{
    Q_OBJECT

public:
    ModuleSTM(QObject* parent);
    ~ModuleSTM();

private:
};

#endif // MODULE_STM_H
