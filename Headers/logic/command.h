#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
//#include <QSize>
#include "Headers/shape_types.h"
#include "Headers/gui/cyclogram/valency_point.h"

class QXmlStreamWriter;
class QXmlStreamReader;

class VariableController;
class SystemState;
class Version;

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

#ifdef ENABLE_CYCLOGRAM_PAUSE
    virtual void pause();
    virtual void resume();
#endif

    virtual bool isVariableUsed(const QString& name) const;

    qint64 id() const;

    void write(QXmlStreamWriter* writer);
    void read(QXmlStreamReader* reader, const Version& fileVersion);

    void setConsoleMessage(const QString& text, uint32_t colorARGB);
    void setConsoleText(const QString& text);
    void setConsoleTextColor(uint32_t argb);

    const QString& text() const;
    const QString& errorDesc() const;
    const QString& consoleText() const;
    QColor consoleTextColor() const;

    DRAKON::IconType type() const;

    const QList<Command*>& nextCommands() const;
    virtual void insertCommand(Command* newCmd, ValencyPoint::Role role);
    void replaceCommand(Command* newCmd, ValencyPoint::Role role = ValencyPoint::Down);
    void replaceCommand(Command* newCmd, Command* oldCmd);

    Command* nextCommand(ValencyPoint::Role role = ValencyPoint::Down) const;

    // TODO remove >>>
    void setRole(ValencyPoint::Role role);
    ValencyPoint::Role role() const;
    // <<<

    uint32_t flags() const;
    bool hasError() const;

    void setFlags(uint32_t flags);
    void setActive(bool active);

    void setExecutionDelay(int msec);
    int executionDelay() const;

    void setVariableController(VariableController* controller);
    VariableController* variableController() const;

    void setSystemState(SystemState* state);
    SystemState* systemState() const;

    bool copyFrom(Command* other);
    virtual bool canBeCopied() const;

signals:
    void finished(Command* nextCmd);
    void dataChanged(const QString& text);
    void errorStatusChanged(bool status); //true - has error, false - no error/error fixed
    void criticalError(Command* cmd); // cmd - where the critical error occured
    void activeStateChanged(bool state); // true - command is active cyclogram cmd, false - become inactive

protected slots:
    virtual void onNameChanged(const QString& newName, const QString& oldName);
    virtual void onVariableRemoved(const QString& name);
    virtual void onVariableAdded(const QString& name, qreal value);

protected:
    void setErrorStatus(bool status); //true - has error, false - no error/error fixed
    void replaceReferences(Command* oldCmd, Command* newCmd, Command* tree);

    virtual void writeCustomAttributes(QXmlStreamWriter* writer);
    virtual void readCustomAttributes(QXmlStreamReader* reader, const Version& fileVersion);
    virtual void updateText();

    virtual bool loadFromImpl(Command* other);

    DRAKON::IconType mType;
    QString mText;
    QString mErrorText;
    QString mConsoleText;
    uint32_t mConsoleTextColor;

    uint32_t mFlags = 0; // Command flags here, by default the command is not interactive

    int mExecutionDelay;
    VariableController* mVarCtrl;
    SystemState* mSystemState;

    QList<Command*> mNextCommands;

// TODO remove >>>
    ValencyPoint::Role mRole; // TODO role belongs to valency point not the command, need to remove
    // correct solution - find command among child shapes commands

private slots:
    void onNextCmdTextChanged(const QString& text);
    void end();

private:
    bool mHasError;
    qint64 mID;
    static qint64 smCounter; // command creation counter in current session
};

#endif // COMMAND_H
