#ifndef CYCLOGRAM_H
#define CYCLOGRAM_H

#include <QObject>

class Command;

class Cyclogram: public QObject
{
    Q_OBJECT

public:
    Cyclogram(QObject * parent);


private:

signals:
};
#endif // CYCLOGRAM_H
