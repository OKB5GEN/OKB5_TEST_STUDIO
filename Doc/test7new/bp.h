#ifndef bp_H
#define bp_H

#include <QObject>
#include <QString>

class bp:public QObject
{
    Q_OBJECT
public:
    bp(QString name);
public slots:
    void bp_avt(int x, int y);
    void bp_timer();
signals:
    void paint();
private:
    QString name;
};

#endif //
