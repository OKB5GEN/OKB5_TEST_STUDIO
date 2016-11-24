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

    virtual QString text() const;
    ShapeTypes type() const;

    const QList<Command*>& nextCommands() const;
    void addCommand(Command* cmd, int role = 0);

    int role() const;
    void setRole(int role);

signals:
    void onFinished(Command* nextCmd); // must be sent on command finish

protected:
    ShapeTypes mType;
    QString mText;
    int mRole;

    QList<Command*> mNextCommands;

private:

};

#endif // COMMAND_H
