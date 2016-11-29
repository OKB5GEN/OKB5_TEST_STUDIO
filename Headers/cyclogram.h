#ifndef CYCLOGRAM_H
#define CYCLOGRAM_H

#include <QObject>
#include <QList>
#include "Headers/cell.h"
#include "Headers/shapetypes.h"

class Command;

class Cyclogram: public QObject
{
    Q_OBJECT

public:
    Cyclogram(QObject * parent);

    void createDefault();

    void run();
    void stop();
    void pause();
    void resume();

    Command* first() const;
    Command* createCommand(ShapeTypes type);

    const QList<Command*>& commands() const;

private slots:
    void onCommandFinished(Command* cmd);

private:
    void clear();
    void deleteCommandTree(Command* cmd);
    void deleteCommand(Command* cmd);
    void createPair(Command* parent, Command* child);

    enum State // TODO correct cyclogram start/pause/stop -ing
    {
        STOPPED,
        RUNNING,
        PAUSED
    };

    Command* mFirst = Q_NULLPTR;
    Command* mLast = Q_NULLPTR;
    Command* mCurrent = Q_NULLPTR;
    State mState = STOPPED;

    QList<Command*> mCommands;

signals:
    void changed();
    void finished();
};
#endif // CYCLOGRAM_H
