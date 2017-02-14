#ifndef COM_PORT_MODULE_H
#define COM_PORT_MODULE_H

#include "Headers/system/abstract_module.h"
#include "Headers/module_commands.h"

#include <QMap>
#include <QVariant>

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

    void initialize();

    virtual void resetError();
    virtual void onApplicationFinish() = 0;

protected:
    struct Request
    {
        uint32_t operation;
        QByteArray data;
        QMap<uint32_t, QVariant> response;
    };

    virtual void processResponse(const QByteArray& response) = 0;
    virtual void initializeCustom() = 0;

    bool send(const QByteArray& request);
    void processQueue();
    void resetPort();

    QSerialPort* mPort;
    Identifier mID;
    QList<Request> mRequestQueue;

private slots:
    void onResponseReceived();
    void onResponseTimeout();

private:
    void createPort(const QString& portName);

    QTimer* mProtectionTimer;
};

#endif // COM_PORT_MODULE_H
