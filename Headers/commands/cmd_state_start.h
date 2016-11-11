#ifndef CMD_STATE_START_H
#define CMD_STATE_START_H

#include "Headers/command.h"

class CmdStateStart: public Command
{
    Q_OBJECT

public:
    CmdStateStart(QString name, QObject * parent);

    void run() override;

    QString getName() const { return mName; }
    void setName(QString name) { mName = name; }

    void setNextCommand(Command* cmd) { mNext = cmd; }

private:
    QString mName;
    Command* mNext = Q_NULLPTR;
};

#endif // CMD_STATE_START_H
