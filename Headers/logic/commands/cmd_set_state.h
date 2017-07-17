#ifndef CMD_SET_STATE_H
#define CMD_SET_STATE_H

#include "Headers/logic/command.h"

class CmdSetState: public Command
{
    Q_OBJECT

public:
    CmdSetState(QObject * parent);

protected:
    bool loadFromImpl(Command* other) override;

public slots:
    void setText(const QString& text);

private:
};

#endif // CMD_SET_STATE_H
