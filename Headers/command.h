#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QSize>

class Command: public QObject
{
    Q_OBJECT

public:
    Command(QObject * parent);
    virtual ~Command();
    virtual void run();
    virtual void stop();
    virtual void pause();
    virtual void resume();

    void setNextCommand(Command* cmd);

signals:
    void onFinished(Command* nextCmd); // must be sent on command finish

protected:
    Command* mNext;

private:

};

#endif // COMMAND_H
