#ifndef MODULE_DRIVE_SIMULATOR_H
#define MODULE_DRIVE_SIMULATOR_H

#include "Headers/system/okb_module.h"

class ModuleDriveSimulator: public ModuleOKB
{
    Q_OBJECT

public:
    ModuleDriveSimulator(QObject* parent);
    ~ModuleDriveSimulator();


public slots:
    void processCustomCommand() override;
    void onApplicationFinish();

protected:
    bool processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response) override;
    void createResponse(Transaction& response) override;
    void onModuleError() override;

private:
    int mSensorsCnt;
};

#endif // MODULE_DRIVE_SIMULATOR_H
