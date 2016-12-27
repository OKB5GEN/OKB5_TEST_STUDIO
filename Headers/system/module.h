#ifndef MODULE_H
#define MODULE_H

#include <QObject>

class QSerialPort;

class Module: public QObject
{
    Q_OBJECT

public:
    Module(QObject* parent);
    virtual ~Module();

    virtual bool init() = 0;

    void setPort(QSerialPort* port);

protected:
    bool send(const QByteArray& request, QByteArray& response);

    QSerialPort* mPort;

private:
};

#endif // MODULE_H
