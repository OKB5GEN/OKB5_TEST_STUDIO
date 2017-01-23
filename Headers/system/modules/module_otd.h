#ifndef MODULE_OTD_H
#define MODULE_OTD_H

#include "Headers/system/okb_module.h"

class QTimer;

class ModuleOTD: public ModuleOKB
{
    Q_OBJECT

public:
    enum LineID
    {
        PSY, // line 1
        NU   // line 2
    };

    ModuleOTD(QObject* parent);
    ~ModuleOTD();

    bool postInitOKBModule() override;

public slots:
    void resetLine(LineID line);
    void readDS1820Data(LineID line);
    void measureDS1820(LineID line);
    void measurePT100();

signals:

private:
    QTimer * m_timer;

    int m_sensorsCntAxis1 = 0;
    int m_sensorsCntAxis2 = 0;
};

#endif // MODULE_OTD_H
