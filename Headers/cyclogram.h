#ifndef CYCLOGRAM_H
#define CYCLOGRAM_H

#include <QObject>
#include <QList>
#include "Headers/cell.h"
#include "Headers/shape_types.h"

class Command;

class Cyclogram: public QObject
{
    Q_OBJECT

public:
    enum State
    {
        STOPPED,
        RUNNING,
        PAUSED
    };

    Cyclogram(QObject * parent);

    void createDefault();

    void run();
    void stop();
    void pause();
    void resume();

    Command* first() const;
    Command* last() const;
    Command* current() const;
    Command* createCommand(DRAKON::IconType type);

    const QList<Command*>& commands() const;
    State state() const;

    void deleteCommand(Command* cmd, bool recursive = false);

    Command* validate() const; //

private slots:
    void onCommandFinished(Command* cmd);
    void runCurrentCommand();

private:
    void clear();
    void deleteCommandTree(Command* cmd);
    void setState(State state);

    Command* mFirst = Q_NULLPTR;
    Command* mLast = Q_NULLPTR;
    Command* mCurrent = Q_NULLPTR;
    State mState = STOPPED;

    QList<Command*> mCommands;

signals:
    void changed();
    void finished();
    void stateChanged(int state);
    void deleted(Command* cmd);
};
#endif // CYCLOGRAM_H
