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

    void resetError() override;

    int softResetModule();
    int getSoftwareVersion();
    bool hasErrors();

public slots:
    void processCommand(const QMap<uint32_t, QVariant>& params) override;

protected:
    void initializeCustom() override;
    virtual void initializeCustomOKBModule();
    virtual void processCustomCommand(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response) = 0;
    virtual void processCustomResponse(const QByteArray& response) = 0;
    virtual void onExecutionError();

    void processResponse(const QByteArray& response) override;

    void addCommandToQueue(ModuleCommands::CommandID cmd, uint8_t param1, uint8_t param2);

    bool mCustomInitializationFinished;

private:
    bool canReturnError(ModuleCommands::CommandID cmd) const;

    uint8_t mCurrentAddress;
    uint8_t mDefaultAddress;
    bool mCommonInitializationFinished;
};

#endif // OKB_MODULE_H
