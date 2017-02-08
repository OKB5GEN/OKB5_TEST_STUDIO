#ifndef MODULE_OTD_H
#define MODULE_OTD_H

#include "Headers/system/okb_module.h"

#include <QList>

class QTimer;

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
    void onApplicationFinish() override;

protected:
    bool postInitOKBModule() override;

private:
    void resetLine(LineID line);
    void readDS1820Data(LineID line);
    void measureDS1820(LineID line, QList<qreal>& values);
    void measurePT100(QList<qreal>& values);

    //QTimer * mTimer;

    int mSensorsCntPsy = 0;
    int mSensorsCntNu = 0;
};

#endif // MODULE_OTD_H
