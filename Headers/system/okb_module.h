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

public slots:
    void processCommand(const Transaction& params) override;

protected:
//    void initializeCustom() override;

    void onTransmissionError(uint32_t operationID) override;
    void onTransmissionComplete() override;

//    virtual void initializeCustomOKBModule();
    virtual void createResponse(Transaction& response);
    virtual void processCustomCommand(const Transaction& request, Transaction& response) = 0;
    virtual bool processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response) = 0;
    virtual void onModuleError() = 0;

    bool processResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response) override;

    void addModuleCmd(ModuleCommands::CommandID cmd, uint8_t param1, uint8_t param2);

private:
    bool canReturnError(ModuleCommands::CommandID cmd) const;

    uint8_t mCurrentAddress;
    uint8_t mDefaultAddress;
};

#endif // OKB_MODULE_H
