#ifndef MODULE_OTD_H
#define MODULE_OTD_H

#include "Headers/system/okb_module.h"

class ModuleOTD: public ModuleOKB
{
    Q_OBJECT

public:
    ModuleOTD(QObject* parent);
    ~ModuleOTD();

public slots:
    void processCustomCommand() override;
    void onApplicationFinish();

protected:
    bool processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response) override;
    void createResponse(Transaction& response) override;
    void onModuleError() override;

private:
    int mSensorsCntPsy;
    int mSensorsCntNu;
};

#endif // MODULE_OTD_H
