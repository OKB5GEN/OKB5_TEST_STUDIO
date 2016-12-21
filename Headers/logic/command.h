#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QSize>
#include "Headers/shape_types.h"
#include "Headers/gui/cyclogram/valency_point.h"

class VariableController;

class Command: public QObject
{
    Q_OBJECT

public:
    enum CommandFlags
    {
        Selectable  = 0x00000001,
        Movable     = 0x00000002,
        Editable    = 0x00000004,
        Deletable   = 0x00000008,

        All = (Selectable | Movable | Editable | Deletable),
    };

    Command(DRAKON::IconType type, int childCmdCnt, QObject * parent);
    virtual ~Command();

    virtual void run();
    virtual void stop();
    virtual void pause();
    virtual void resume();

    const QString& text() const;
    const QString& errorDesc() const;
    DRAKON::IconType type() const;

    const QList<Command*>& nextCommands() const;
    void insertCommand(Command* newCmd, ValencyPoint::Role role = ValencyPoint::Down);
    void replaceCommand(Command* newCmd, ValencyPoint::Role role = ValencyPoint::Down);

    Command* nextCommand(ValencyPoint::Role role = ValencyPoint::Down) const;

    // TODO remove >>>
    Command* parentCommand() const;
    void setParentCommand(Command* cmd);
    // <<<

    ValencyPoint::Role role() const;
    uint32_t flags() const;

    bool hasError() const;

    void setRole(ValencyPoint::Role role);
    void setFlags(uint32_t flags);
    void setActive(bool active);

    void setExecutionDelay(int msec);

    void setVariableController(VariableController* controller);
    VariableController* variableController() const;

signals:
    void finished(Command* nextCmd);
    void textChanged(const QString& text);
    void errorStatusChanged(bool status); //true - has error, false - no error/error fixed
    void criticalError(Command* cmd); // cmd - where the critical error occured
    void activeStateChanged(bool state); // true - command is active cyclogram cmd, false - become inactive

protected slots:
    virtual void onNameChanged(const QString& newName, const QString& oldName);
    virtual void onVariableRemoved(const QString& name);

protected:
    void setErrorStatus(bool status); //true - has error, false - no error/error fixed

    DRAKON::IconType mType;
    QString mText;
    QString mErrorText;
    ValencyPoint::Role mRole;
    uint32_t mFlags = 0; // Command flags here, by default the command is not interactive

    int mExecutionDelay;
    VariableController* mVarCtrl;

    QList<Command*> mNextCommands;

// TODO remove >>>
    Command* mParentCommand; // TODO BRANCH_BEGIN has not parent command. REFACTOR - many commands can be "parent"

private slots:
    void onNextCmdTextChanged(const QString& text);
    void end();

private:
    void replaceReferences(Command* oldCmd, Command* newCmd, Command* tree);

    bool mHasError;
};

#endif // COMMAND_H
