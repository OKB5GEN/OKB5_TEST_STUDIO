#ifndef CMD_SET_STATE_H
#define CMD_SET_STATE_H

#include "Headers/command.h"

class CmdSetState: public Command
{
    Q_OBJECT

public:
    CmdSetState(QString name, QObject * parent);

    QString getName() const { return mName; }
    void setName(QString name) { mName = name; }
private:
    QString mName;
};

#endif // CMD_SET_STATE_H
