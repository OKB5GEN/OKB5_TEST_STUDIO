#ifndef MODULE_DRIVE_SIMULATOR_H
#define MODULE_DRIVE_SIMULATOR_H

#include "Headers/system/okb_module.h"

#include <QList>

class ModuleDriveSimulator: public ModuleOKB
{
    Q_OBJECT

public:
    ModuleDriveSimulator(QObject* parent);
    ~ModuleDriveSimulator();

    int sensorsCount() const;

public slots:
    void processCustomCommand() override;
    void onApplicationFinish();

protected:
    bool processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response) override;
    void createResponse(Transaction& response) override;
    void onModuleError() override;

private:
    int mSensorsCnt;

    QList<qreal> mTemperatureData;
};

#endif // MODULE_DRIVE_SIMULATOR_H
