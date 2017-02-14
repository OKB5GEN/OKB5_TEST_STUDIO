#ifndef MODULE_OTD_H
#define MODULE_OTD_H

#include "Headers/system/okb_module.h"

#include <QList>

class ModuleOTD: public ModuleOKB
{
    Q_OBJECT

public:
    enum LineID
    {
        PSY = 1, // line 1
        NU  = 2  // line 2
    };

    ModuleOTD(QObject* parent);
    ~ModuleOTD();

    int ptCount() const;
    int dsCount(LineID line) const;

public slots:
    void processCustomCommand(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response) override;
    void processCustomResponse(const QByteArray& response) override;
    void onApplicationFinish() override;

protected:
    void initializeCustomOKBModule() override;

private:
    int mSensorsCntPsy = 0;
    int mSensorsCntNu = 0;

    QList<qreal> mTemperatureData;
};

#endif // MODULE_OTD_H
