#ifndef OKB_MODULE_H
#define OKB_MODULE_H

#include "Headers/system/com_port_module.h"

class ModuleOKB: public COMPortModule
{
    Q_OBJECT

public:
    ModuleOKB(QObject* parent);
    virtual ~ModuleOKB();

    uint8_t defaultAddress() const;
    uint8_t currentAddress() const;

    bool postInit() override;
    void resetError() override;

    int softResetModule();
    int getSoftwareVersion();
    QString checkStatusWord();

protected:
    bool send(ModuleCommands::CommandID cmd, uint8_t param1, uint8_t param2);

    uint8_t mAddress;
    uint8_t mDefaultAddress;

signals:
    void incorrectSlot(uint8_t defaultAddr);

private:
    bool canReturnError(ModuleCommands::CommandID cmd) const;
};

#endif // OKB_MODULE_H
