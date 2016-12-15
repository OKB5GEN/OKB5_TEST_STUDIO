#ifndef CYCLOGRAM_H
#define CYCLOGRAM_H

#include <QObject>
#include <QList>

#include "Headers/shape_types.h"

class Command;
class VariableController;

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

    Q_ENUM(State)

    Cyclogram(QObject * parent);

    void createDefault();

    void run();
    void stop();
    void pause();
    void resume();

    //void setExecuteOneCmd(bool enable);

    Command* first() const;
    Command* last() const;
    Command* current() const;
    Command* createCommand(DRAKON::IconType type, int param = -1);

    const QList<Command*>& commands() const;
    void getBranches(QList<Command*>& branches) const;

    State state() const;

    void deleteCommand(Command* cmd, bool recursive = false);

    Command* validate() const; //

    VariableController* varCtrl() const;

private slots:
    void onCommandFinished(Command* cmd);
    void onCriticalError(Command* cmd);
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

    VariableController* mVarController;

    //bool mExecuteOneCmd;

signals:
    void changed();
    void finished(const QString& error);
    void stateChanged(int state);
    void deleted(Command* cmd);
};
#endif // CYCLOGRAM_H
