#ifndef CMD_ACTION_MODULE_H
#define CMD_ACTION_MODULE_H

#include "Headers/logic/commands/cmd_action.h"
#include "Headers/module_commands.h"

class CmdActionModule: public CmdAction
{
    Q_OBJECT

public:
    CmdActionModule(QObject* parent);

    void run() override;

    void setOperation(ModuleCommands::ModuleID module, ModuleCommands::CommandID operation);

    ModuleCommands::CommandID operation() const;
    ModuleCommands::ModuleID module() const;

/*
    void setOperand(OperandID operand, qreal value);
    void setOperand(OperandID operand, const QString& variable);
    OperandType operandType(OperandID operand) const;
    QString variableName(OperandID operand) const;
    qreal value(OperandID operand) const;*/

private slots:
//    void onNameChanged(const QString& newName, const QString& oldName) override;
//    void onVariableRemoved(const QString& name) override;
    void execute();

private:
    void updateText();

    ModuleCommands::ModuleID mModule;
    ModuleCommands::CommandID mOperation;
};
#endif // CMD_ACTION_MODULE_H
