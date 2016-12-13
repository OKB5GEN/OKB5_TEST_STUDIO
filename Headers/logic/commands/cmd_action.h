#ifndef CMD_ACTION_H
#define CMD_ACTION_H

#include "Headers/command.h"

class CmdAction: public Command
{
    Q_OBJECT

public:
    CmdAction(DRAKON::IconType type, QObject* parent);

    void run() override;

protected slots:
    void finish();

protected:


private:
};
#endif // CMD_ACTION_H
