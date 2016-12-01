#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QSize>
#include "Headers/shapetypes.h"

class Command: public QObject
{
    Q_OBJECT

public:
    enum CommandFlags
    {
        Selectable  = 0x00000001,
        Movable     = 0x00000002,
        Editable    = 0x00000004,
        Deletable   = 0x00000080,

        All = (Selectable | Movable | Editable | Deletable),
    };

    Command(DRAKON::IconType type, QObject * parent);
    virtual ~Command();

    virtual void run();
    virtual void stop();
    virtual void pause();
    virtual void resume();

    virtual QString text() const;
    DRAKON::IconType type() const;

    const QList<Command*>& nextCommands() const;
    void addCommand(Command* cmd, int role = 0);
    void insertCommand(Command* newCmd, int role = 0);
    void replaceCommand(Command* newCmd, int role = 0);

    int role() const;
    uint32_t flags() const;

    void setRole(int role);
    void setFlags(uint32_t flags);

signals:
    void finished(Command* nextCmd); // must be sent on command finish
    void textChanged(const QString& text);

protected:
    DRAKON::IconType mType;
    QString mText;
    int mRole;
    uint32_t mFlags = 0; // Command flags here, by default the command is not interactive

    QList<Command*> mNextCommands;

private slots:
    void onNextCmdTextChanged(const QString& text);

private:
};

#endif // COMMAND_H
