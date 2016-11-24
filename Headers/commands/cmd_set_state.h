#ifndef CMD_SET_STATE_H
#define CMD_SET_STATE_H

#include "Headers/command.h"

class CmdSetState: public Command
{
    Q_OBJECT

public:
    CmdSetState(const QString& name, QObject * parent);

    const QString& name() const;
    void setName(const QString& name);
private:
    QString mName;
};

#endif // CMD_SET_STATE_H
