#ifndef CMD_ACTION_MODULE_H
#define CMD_ACTION_MODULE_H

#include "Headers/logic/commands/cmd_action.h"

class CmdActionModule: public CmdAction
{
    Q_OBJECT

public:
    enum Module
    {
        POWER_UNIT_BUP,
        POWER_UNIT_PNA,
        MKO,
        STM,
        TECH,
        OTD
    };

    enum Operation
    {
        // Power unit operations
        SET_VOLTAGE_AND_CURRENT,
        SET_MAX_VOLTAGE_AND_CURRENT,
        SET_POWER_STATE,
        GET_CURRENT_VOLTAGE_AND_CURRENT,

        //
    };

    CmdActionModule(QObject* parent);

    void run() override;

    void setOperation(Module module, Operation operation);

    Operation operation() const;
    Module module() const;

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

    Module mModule;
    Operation mOperation;

};
#endif // CMD_ACTION_MODULE_H
