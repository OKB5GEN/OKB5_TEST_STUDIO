#ifndef COM_PORT_MODULE_H
#define COM_PORT_MODULE_H

#include "Headers/system/abstract_module.h"
#include "Headers/module_commands.h"

class QSerialPort;

class COMPortModule: public AbstractModule
{
    Q_OBJECT

public:
    struct Identifier
    {
        QString description;
        QString manufacturer;
        QString serialNumber;
        quint16 vendorId;
        quint16 productId;
    };

    COMPortModule(QObject* parent);
    virtual ~COMPortModule();

    void setId(const Identifier& id);
    const Identifier& id() const;

    bool init();

    virtual bool postInit() = 0;
    virtual void resetError();

protected:
    bool send(const QByteArray& request, QByteArray& response);

    void resetPort();

    QSerialPort* mPort;
    Identifier mID;

private:
    void createPort(const QString& portName);
};

#endif // COM_PORT_MODULE_H
