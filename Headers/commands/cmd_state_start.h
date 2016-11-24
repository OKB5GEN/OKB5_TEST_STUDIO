#ifndef CMD_STATE_START_H
#define CMD_STATE_START_H

#include "Headers/command.h"

class CmdStateStart: public Command
{
    Q_OBJECT

public:
    CmdStateStart(const QString& name, QObject * parent);

    void run() override;

    const QString& name() const;
    void setName(const QString& name);

private:
    QString mName;
};

#endif // CMD_STATE_START_H
