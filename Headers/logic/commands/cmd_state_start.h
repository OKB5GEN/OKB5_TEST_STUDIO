#ifndef CMD_STATE_START_H
#define CMD_STATE_START_H

#include "Headers/command.h"

class CmdStateStart: public Command
{
    Q_OBJECT

public:
    CmdStateStart(QObject * parent);

public slots:
    void setText(const QString& text);

private:
};

#endif // CMD_STATE_START_H
