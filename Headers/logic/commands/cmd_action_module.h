#ifndef CMD_ACTION_MODULE_H
#define CMD_ACTION_MODULE_H

#include "Headers/logic/commands/cmd_action.h"

class CmdActionModule: public CmdAction
{
    Q_OBJECT

public:
/*    enum Operation
    {
        Add,        // +
        Subtract,   // -
        Multiply,   // *
        Divide,     // :
        Assign      // = by default
    };

    enum OperandID
    {
        Result,
        Operand1,
        Operand2,

        OperandsCount
    };

    enum OperandType
    {
        Variable,
        Number,

        OperandNotSet
    };*/

    CmdActionModule(QObject* parent);

    void run() override;

/*    void setOperation(Operation operation);
    void setOperand(OperandID operand, qreal value);
    void setOperand(OperandID operand, const QString& variable);

    Operation operation() const;
    OperandType operandType(OperandID operand) const;
    QString variableName(OperandID operand) const;
    qreal value(OperandID operand) const;*/

private slots:
//    void onNameChanged(const QString& newName, const QString& oldName) override;
//    void onVariableRemoved(const QString& name) override;
    void execute();

private:
    void updateText();
};
#endif // CMD_ACTION_MODULE_H
