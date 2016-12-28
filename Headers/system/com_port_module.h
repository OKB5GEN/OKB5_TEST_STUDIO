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

    virtual void postInit() = 0;

    bool init(QSerialPort* port);

    void setPort(QSerialPort* port);
    uint8_t defaultAddress() const;
    uint8_t currentAddress() const;

protected:
    bool send(ModuleCommands::CommandID cmd, uint8_t param1 = 0, uint8_t param2 = 0);

    QSerialPort* mPort;

    uint8_t mAddress;
    uint8_t mDefaultAddress;

signals:
    void incorrectSlot(uint8_t defaultAddr);

private:
    bool send(const QByteArray& request, QByteArray& response);
    bool canReturnError(ModuleCommands::CommandID cmd) const;
};

#endif // MODULE_H
