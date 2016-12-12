#ifndef CMD_QUESTION_H
#define CMD_QUESTION_H

#include "Headers/command.h"

class CmdQuestion: public Command
{
    Q_OBJECT

public:
    enum Role
    {
        Yes,
        No,
        AfterArrow
    };

    enum Operation
    {
        Greater,        // >
        Less,           // <
        Equal,          // ==
        GreaterOrEqual, // >=
        LessOrEqual,    // <=
        NotEqual        // !=
    };

    enum OperandID
    {
        Left,
        Right,

        OperandsCount
    };

    enum OperandType
    {
        Variable,
        Number,

        OperandNotSet
    };

    CmdQuestion(QObject* parent);

    void run() override;

    void setOperation(Operation operation);
    void setOperand(OperandID operand, qreal value);
    void setOperand(OperandID operand, const QString& variable);

    Operation operation() const;
    OperandType operandType(OperandID operand) const;
    QString variableName(OperandID operand) const;
    qreal value(OperandID operand) const;

    void swapBranches();

private slots:
    void onNameChanged(const QString& newName, const QString& oldName) override;
    void onVariableRemoved(const QString& name) override;
    void execute();

private:
    struct OperandData
    {
        OperandData();

        QString variable;
        qreal value;
        OperandType type;
    };

    void updateText();

    Operation mOperation;
    OperandData mOperands[OperandsCount];
};
#endif // CMD_QUESTION_H
