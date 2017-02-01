#ifndef OKB_MODULE_H
#define OKB_MODULE_H

#include "Headers/system/com_port_module.h"

class ModuleOKB: public COMPortModule
{
    Q_OBJECT

public:
    enum StatusMask
    {
        MODULE_READY_MASK = 0x80,
        HAS_ERRORS_MASK   = 0x40,
        AFTER_RESET_MASK  = 0x20,
    };

    ModuleOKB(QObject* parent);
    virtual ~ModuleOKB();

    uint8_t defaultAddress() const;
    uint8_t currentAddress() const;

    void resetError() override;

    int softResetModule();
    int getSoftwareVersion();
    bool hasErrors();

public slots:
    void processCommand(const QMap<uint32_t, QVariant>& params) override;

protected:
    bool postInit() override;
    virtual bool postInitOKBModule();
    virtual void processCustomCommand(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response) = 0;

    bool sendCommand(ModuleCommands::CommandID cmd, uint8_t param1, uint8_t param2, QByteArray* response = Q_NULLPTR);

    uint8_t mAddress;
    uint8_t mDefaultAddress;

signals:
    void incorrectSlot(uint8_t defaultAddr);

private:
    bool canReturnError(ModuleCommands::CommandID cmd) const;
};

#endif // OKB_MODULE_H
