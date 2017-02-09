#ifndef CYCLOGRAM_H
#define CYCLOGRAM_H

#include <QObject>
#include <QList>

#include "Headers/shape_types.h"

class Command;
class VariableController;
class SystemState;

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

    bool isModified() const;
    void setModified(bool isModified, bool sendSignal);

    //void setExecuteOneCmd(bool enable);

    Command* first() const;
    Command* last() const;
    Command* current() const;
    Command* createCommand(DRAKON::IconType type, int param = -1);

    void setFirst(Command* first);
    void setLast(Command* last);

    const QList<Command*>& commands() const;
    void getBranches(QList<Command*>& branches) const;

    State state() const;

    void deleteCommand(Command* cmd, bool recursive = false);

    Command* validate() const; //

    VariableController* variableController() const;
    void setSystemState(SystemState* state);

    static bool isCyclogramEndBranch(Command* cmd);

    void clear(); //TODO possibly temporary

private slots:
    void onCommandFinished(Command* cmd);
    void onCriticalError(Command* cmd);
    void runCurrentCommand();
    void onCommandTextChanged(const QString& text);
    void variablesChanged();
    void variableValueChanged(const QString& name, qreal value, int container);

private:
    void deleteCommandTree(Command* cmd, bool silent);
    void deleteCommandImpl(Command* cmd, bool silent);
    void setState(State state);

    Command* mFirst = Q_NULLPTR;
    Command* mLast = Q_NULLPTR;
    Command* mCurrent = Q_NULLPTR;
    State mState = STOPPED;

    QList<Command*> mCommands;

    VariableController* mVarController;
    SystemState* mSystemState;

    //bool mExecuteOneCmd;
    bool mModified;

signals:
    void changed();
    void finished(const QString& error);
    void stateChanged(int state);
    void deleted(Command* cmd);

    void modified(); //TODO send signal + vs "changed" signal
};
#endif // CYCLOGRAM_H
