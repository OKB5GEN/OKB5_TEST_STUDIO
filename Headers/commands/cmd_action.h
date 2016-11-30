#ifndef CMD_ACTION_H
#define CMD_ACTION_H

#include "Headers/command.h"

class CmdAction: public Command
{
    Q_OBJECT

public:
    CmdAction(QObject* parent);

    void run() override;
    void stop() override;
    void pause() override;
    void resume() override;

private slots:
    void finish();

private:
};
#endif // CMD_ACTION_H
