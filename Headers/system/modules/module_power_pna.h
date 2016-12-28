#ifndef MODULE_POWER_PNA_H
#define MODULE_POWER_PNA_H

#include <QObject>

class ModulePowerPNA: public QObject
{
    Q_OBJECT

public:
    ModulePowerPNA(QObject* parent);
    ~ModulePowerPNA();

private:
};

#endif // MODULE_POWER_PNA_H
