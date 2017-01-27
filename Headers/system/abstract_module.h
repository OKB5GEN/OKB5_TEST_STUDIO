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
public slots:
    virtual void processCommand(const QMap<uint32_t, QVariant>&) = 0;

signals:
    void commandResult(const QMap<uint32_t, QVariant>&);
};

#endif // ABSTRACT_MODULE_H
