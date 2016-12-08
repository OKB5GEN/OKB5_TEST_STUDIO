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

protected slots:
    void finish();
    virtual void onNameChanged(const QString& newName, const QString& oldName);
    virtual void onVariableRemoved(const QString& name);

protected:
    VariableController* mVarCtrl;

private:
};
#endif // CMD_ACTION_H
