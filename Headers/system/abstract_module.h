#ifndef ABSTRACT_MODULE_H
#define ABSTRACT_MODULE_H

#include <QObject>

class AbstractModule: public QObject
{
    Q_OBJECT

public:
    AbstractModule(QObject* parent): QObject(parent)
    {
    }

    //TODO store module type for logging purposes

public slots:
    virtual void processCommand(const QMap<uint32_t, QVariant>& request) = 0;
    virtual void setDefaultState() = 0;

signals:
    void commandResult(const QMap<uint32_t, QVariant>& response);
    void initializationFinished(const QString& error); // empty string - OK, not empty - initialization failed (error description)
};

#endif // ABSTRACT_MODULE_H
