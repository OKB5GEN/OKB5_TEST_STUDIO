#ifndef MODULE_H
#define MODULE_H

#include <QObject>
#include "Headers/module_commands.h"

class QSerialPort;

class COMPortModule: public QObject
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

#endif // MODULE_H
