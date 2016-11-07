#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>

class Command: public QObject
{
    Q_OBJECT

public:
    Command(QObject * parent);
    virtual ~Command();
    virtual void run();
    virtual void stop();

signals:
    void onFinished(Command* cmd); // must be sent on command finish

private:
};

#endif // COMMAND_H
