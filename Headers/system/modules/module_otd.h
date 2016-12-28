#ifndef MODULE_OTD_H
#define MODULE_OTD_H

#include "Headers/system/module.h"

class ModuleOTD: public Module
{
    Q_OBJECT

public:
    enum LineID
    {
        PSY, // line 1
        NU   // line 2
    };

    ModuleOTD(QObject* parent);
    ~ModuleOTD();

    void resetLine(LineID line);

    void postInit() override;

private:
};

#endif // MODULE_OTD_H
