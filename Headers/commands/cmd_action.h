#ifndef CMD_ACTION_H
#define CMD_ACTION_H

#include "Headers/command.h"

class VariableController;

class CmdAction: public Command
{
    Q_OBJECT

public:
    CmdAction(DRAKON::IconType type, QObject* parent);

    void setVariableController(VariableController* controller);
    VariableController* variableController() const;

    void run() override;
    void stop() override;
    void pause() override;
    void resume() override;

protected slots:
    void finish();

protected:
    VariableController* mVarCtrl;

private:
};
#endif // CMD_ACTION_H
