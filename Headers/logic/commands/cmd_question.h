#ifndef CMD_QUESTION_H
#define CMD_QUESTION_H

#include "Headers/logic/command.h"

class CmdQuestion: public Command
{
    Q_OBJECT

public:
    enum Orientation
    {
        YesDown,
        YesRight
    };

    Q_ENUM(Orientation)

    enum Operation
    {
        Greater,        // >
        Less,           // <
        Equal,          // ==
        GreaterOrEqual, // >=
        LessOrEqual,    // <=
        NotEqual        // !=
    };

    Q_ENUM(Operation)

    enum OperandID
    {
        Left,
        Right,

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

    struct OperandData // TODO replace by QVariant
    {
        OperandData();

        QString variable;
        qreal value;
        OperandType type;
    };

    CmdQuestion(int commandID, int pointsCount, QObject* parent);

    void run() override;

    void setData(Operation operation, Orientation orientation, const OperandData& left, const OperandData& right);

    void setOperation(Operation operation);
    void setOrientation(Orientation orientation);
    void setOperand(OperandID operand, qreal value);
    void setOperand(OperandID operand, const QString& variable);

    Operation operation() const;
    Orientation orientation() const;
    OperandType operandType(OperandID operand) const;
    QString variableName(OperandID operand) const;
    qreal value(OperandID operand) const;

    bool canBeCopied() const override;

protected:
    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;
    bool loadFromImpl(Command* other) override;
    void updateText() override;

private slots:
    void onNameChanged(const QString& newName, const QString& oldName) override;
    void onVariableRemoved(const QString& name) override;
    void execute();

private:
    Operation mOperation;
    Orientation mOrientation;
    OperandData mOperands[OperandsCount];
};
#endif // CMD_QUESTION_H
