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

    static double getDS1820Temp(const QByteArray& response);
    static double getPT100Temp(const QByteArray& response);

public slots:
    void processCommand(const Transaction& request) override;

protected:
    void onTransmissionError(uint32_t operationID) override;
    void onTransmissionComplete() override;

    virtual void createResponse(Transaction& response);
    virtual void processCustomCommand() = 0;
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
