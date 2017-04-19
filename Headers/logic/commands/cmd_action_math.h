#ifndef CMD_ACTION_MATH_H
#define CMD_ACTION_MATH_H

#include "Headers/logic/commands/cmd_action.h"

class CmdActionMath: public CmdAction
{
    Q_OBJECT

public:
    enum Operation
    {
        Add,        // +
        Subtract,   // -
        Multiply,   // *
        Divide,     // :
        Assign      // = by default
    };

    Q_ENUM(Operation)

    enum OperandID
    {
        Result,
        Operand1,
        Operand2,

        OperandsCount
    };

    Q_ENUM(OperandID)

    enum OperandType
    {
        Variable,
        Number,

        OperandNotSet
    };

    Q_ENUM(OperandType)

    CmdActionMath(QObject* parent);

    void run() override;

    void setOperation(Operation operation);
    void setOperand(OperandID operand, qreal value);
    void setOperand(OperandID operand, const QString& variable);

    Operation operation() const;
    OperandType operandType(OperandID operand) const;
    QString variableName(OperandID operand) const;
    qreal value(OperandID operand) const;

private slots:
    void onNameChanged(const QString& newName, const QString& oldName) override;
    void onVariableRemoved(const QString& name) override;
    void execute();

private:
    struct OperandData //TODO replace by QVariant usage, similar to CmdActionModule
    {
        OperandData();

        QString variable;
        qreal value;
        OperandType type;
    };

    void updateText() override;

    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;

    Operation mOperation;
    OperandData mOperands[OperandsCount];
};
#endif // CMD_ACTION_MATH_H
