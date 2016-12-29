#ifndef MODULE_H
#define MODULE_H

#include <QObject>
#include "Headers/module_commands.h"

class QSerialPort;

class COMPortModule: public QObject
{
    Q_OBJECT

public:
    COMPortModule(QObject* parent);
    virtual ~COMPortModule();

    bool init();

    virtual bool postInit() = 0;
    virtual void resetError();

protected:
    bool send(const QByteArray& request, QByteArray& response);

    void resetPort();

    QSerialPort* mPort;

private:
    void createPort();
};

#endif // MODULE_H
