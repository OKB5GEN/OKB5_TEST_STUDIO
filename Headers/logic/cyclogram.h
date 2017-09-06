#ifndef CYCLOGRAM_H
#define CYCLOGRAM_H

#include <QVariant>

#include "Headers/shape_types.h"

class Command;
class VariableController;
class SystemState;
class ValencyPoint;

class Cyclogram: public QObject
{
    Q_OBJECT

public:
    static const QString SETTING_DESCRIPTION;
    static const QString SETTING_CLEANUP_CYCLOGRAM;
    static const QString SETTING_DEFAULT_NAME;

    enum State
    {
        IDLE,
        RUNNING,
        PENDING_FOR_START,
#ifdef ENABLE_CYCLOGRAM_PAUSE
        PAUSED
#endif
    };

    Q_ENUM(State)

    Cyclogram(QObject * parent);

    static QString defaultStorePath();

    void createDefault();

    void run();
    void stop();
#ifdef ENABLE_CYCLOGRAM_PAUSE
    void pause();
    void resume();
#endif

    bool isModified() const;
    void setModified(bool isModified, bool sendSignal, bool recursive);

    Command* first() const;
    Command* last() const;
    Command* current() const;
    Command* createCommand(DRAKON::IconType type);
    Command* createBranchCopy(Command* branch);

    QString generateBranchName(const QString& templateName) const;

    void setFirst(Command* first);
    void setLast(Command* last);

    const QList<Command*>& commands() const;
    void getBranches(QList<Command*>& branches) const;

    State state() const;

    void deleteCommand(Command* cmd, bool recursive = false);
    void deleteBranch(Command* cmd);

    Command* validate() const; //

    VariableController* variableController() const;
    void setSystemState(SystemState* state);
    SystemState* systemState() const;

    static bool isCyclogramEndBranch(Command* cmd);
    static bool canBeCopied(DRAKON::IconType type);

    void clear();

    bool isMainCyclogram() const;
    void setMainCyclogram(bool isMain);

    void changeCommandsOrder(Command* commandToMove, Command* commandAfter);

    // cyclogram settings
    QVariant setting(const QString& name) const;
    void setSetting(const QString& key, const QVariant& value, bool sendSignal = true);
    const QMap<QString, QVariant>& settings() const;

    void setState(State state);

private slots:
    void onCommandFinished(Command* cmd);
    void onCriticalError(Command* cmd);
    void runCurrentCommand();
    void onCommandTextChanged(const QString& text);
    void variablesChanged();
    void variableCurrentValueChanged(const QString& name, qreal value);
    void variableInitialValueChanged(const QString& name, qreal value);
    void onAppSettingsChanged();

private:
    void deleteCommandTree(Command* cmd, bool silent, bool isBranchDeletion);
    void deleteCommandImpl(Command* cmd, bool silent);
    void copyCommandTree(Command* to, Command* from, QMap<Command*, Command*>& alreadyCreatedCommands);
    Command* copyCommand(Command* to, Command* from, int role, QMap<Command*, Command*>& alreadyCreatedCommands, bool addToMapping);

    Command* mFirst;
    Command* mLast;
    Command* mCurrent;
    State mState;

    QList<Command*> mCommands;

    VariableController* mVarController;
    SystemState* mSystemState;
    bool mIsMainCyclogram;
    bool mModified;

    QMap<QString, QVariant> mSettings;

signals:
    void commandStarted(Command* cmd);
    void commandFinished(Command* cmd);
    void finished(const QString& error);
    void stateChanged(int state);
    void deleted(Command* cmd);

    void modified();
};
#endif // CYCLOGRAM_H
