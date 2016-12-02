#ifndef CMD_SET_STATE_H
#define CMD_SET_STATE_H

#include "Headers/command.h"

class CmdSetState: public Command
{
    Q_OBJECT

public:
    CmdSetState(QObject * parent);

public slots:
    void setText(const QString& text);

private:
};

#endif // CMD_SET_STATE_H
