#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include "Headers/logic/variable_controller.h"

class QSerialPort;

class SystemState: public VariableController
{
    Q_OBJECT

public:
    SystemState(QObject* parent);
    ~SystemState();

    void init();

private:
    QSerialPort * createPort();
};
#endif // SYSTEM_STATE_H
