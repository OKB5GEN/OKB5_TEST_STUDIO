#ifndef MODULE_DRIVE_SIMULATOR_H
#define MODULE_DRIVE_SIMULATOR_H

#include "Headers/system/okb_module.h"

#include <QList>

class ModuleDriveSimulator: public ModuleOKB
{
    Q_OBJECT

public:
    enum LineID
    {
        PSY = 1, // line 1
        NU  = 2  // line 2
    };

    ModuleDriveSimulator(QObject* parent);
    ~ModuleDriveSimulator();

    int ptCount() const;
    int dsCount(LineID line) const;

public slots:
    void processCustomCommand() override;
    void onApplicationFinish();

protected:
    bool processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response) override;
    void createResponse(Transaction& response) override;
    void onModuleError() override;

private:
    int mSensorsCntPsy = 0;
    int mSensorsCntNu = 0;

    QList<qreal> mTemperatureData;
};

#endif // MODULE_DRIVE_SIMULATOR_H
