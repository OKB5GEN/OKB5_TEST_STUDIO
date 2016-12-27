#ifndef MYCLASS_H
#define MYCLASS_H

#include <QObject>
#include <QString>

class MyClass:public QObject
{
Q_OBJECT
public:
    MyClass(QString name);
public slots:
    void doWork();
signals:
    void send();
    void send2();
    void send3();
    void send4();
    void send5();
private:
    QString name;
};

#endif // MYCLASS_H
