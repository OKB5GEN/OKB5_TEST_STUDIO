#ifndef MODULE_H
#define MODULE_H

#include <QObject>

class Module: public QObject
{
    Q_OBJECT

public:
    Module(QObject* parent);
    virtual ~Module();

private:
};

#endif // MODULE_H
