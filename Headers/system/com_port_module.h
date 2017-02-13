#ifndef COM_PORT_MODULE_H
#define COM_PORT_MODULE_H

#include "Headers/system/abstract_module.h"
#include "Headers/module_commands.h"

class QSerialPort;
class QTimer;

class COMPortModule: public AbstractModule
{
    Q_OBJECT

public:
    struct Identifier
    {
        QString description;
        QString serialNumber;
        quint16 vendorId;
        quint16 productId;
    };

    COMPortModule(QObject* parent);
    virtual ~COMPortModule();

    void setId(const Identifier& id);
    const Identifier& id() const;

    bool initialize();

    virtual bool postInit() = 0;
    virtual void resetError();
    virtual void onApplicationFinish() = 0;

protected:
    bool send(const QByteArray& request);

    virtual void processResponse(const QByteArray& response) = 0;

    void resetPort();

    QSerialPort* mPort;
    Identifier mID;
    bool mIsInitialized;

private slots:
    void onResponseReceived();
    void onResponseTimeout();

private:
    void createPort(const QString& portName);

    QTimer* mProtectionTimer;
};

#endif // COM_PORT_MODULE_H
