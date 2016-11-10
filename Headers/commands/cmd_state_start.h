#ifndef CMD_STATE_START_H
#define CMD_STATE_START_H

#include "Headers/command.h"

class CmdStateStart: public Command
{
    Q_OBJECT

public:
    CmdStateStart(QString name, QObject * parent);

    QString getName() const { return mName; }
    void setName(QString name) { mName = name; }
private:
    QString mName;
};

#endif // CMD_STATE_START_H
