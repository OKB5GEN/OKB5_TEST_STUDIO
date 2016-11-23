#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QSize>
#include "Headers/shapetypes.h"

class Command: public QObject
{
    Q_OBJECT

public:
    Command(ShapeTypes type, QObject * parent);
    virtual ~Command();

    virtual void run();
    virtual void stop();
    virtual void pause();
    virtual void resume();

    void setNext(Command* cmd);
    ShapeTypes type() const;

signals:
    void onFinished(Command* nextCmd); // must be sent on command finish

protected:
    Command* mNext;
    ShapeTypes mType;


private:

};

#endif // COMMAND_H
