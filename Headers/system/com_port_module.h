#ifndef COM_PORT_MODULE_H
#define COM_PORT_MODULE_H

#include <QtSerialPort>

#include "Headers/system/abstract_module.h"
#include "Headers/module_commands.h"

//class QSerialPort;
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

    void setSendInterval(int msec);
    void setResponseWaitTime(int msec);

    void setId(ModuleCommands::ModuleID moduleID, const Identifier& id);
    const Identifier& id() const;

    void initialize();

    // callback for some actions that must be performed on application finish
    virtual void onApplicationFinish() = 0;

protected:
    // return true if processing successful
    // return false if processing failed, and request queue must be resetted
    virtual bool processResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response) = 0;

    // called when current request was not send to COM port or response not received
    virtual void onTransmissionError(uint32_t operationID) = 0;

    // call when all requests in queue are succesfully sent and all responses to them are successfully processed
    virtual void onTransmissionComplete() = 0;

    // inherited class custom initialization. The signal 'initializationFinished' must be sent after initialization finished
    virtual void initializeCustom() = 0;

    virtual void onSoftResetComplete() = 0;

    void addRequest(uint32_t operationID, const QByteArray& request);
    void softReset();

    ModuleCommands::ModuleID moduleID() const;
    const QString& moduleName() const;

    bool mModuleReady;

private slots:
    void onResponseReceived();
    void onResponseTimeout();
    void sendRequest();
    void tryCreatePort();
    void onErrorOccured(QSerialPort::SerialPortError error);

private:
    struct Request
    {
        uint32_t operation;
        QByteArray data;
    };

    bool sendToPort(const QByteArray& request);
    QString createPort(const QString& portName);
    QString findPortName() const;

    QSerialPort* mPort;
    Identifier mID;
    ModuleCommands::ModuleID mModuleID;
    QString mModuleName;

    QTimer* mResponseWaitTimer;
    int mResponseWaitTime;

    QTimer* mSendTimer;
    int mSendInterval;

    QTimer* mSoftResetTimer;

    QList<Request> mRequestQueue;
};

#endif // COM_PORT_MODULE_H
