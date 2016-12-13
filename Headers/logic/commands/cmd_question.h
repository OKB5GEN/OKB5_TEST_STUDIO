#ifndef CMD_QUESTION_H
#define CMD_QUESTION_H

#include "Headers/logic/command.h"

class CmdQuestion: public Command
{
    Q_OBJECT

public:
    enum QuestionType
    {
        IF,
        CYCLE
    };

    enum Orientation
    {
        YesDown,
        YesRight
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

    void setQuestionType(QuestionType type);
    void setOperation(Operation operation);
    void setOrientation(Orientation operation);
    void setOperand(OperandID operand, qreal value);
    void setOperand(OperandID operand, const QString& variable);

    Operation operation() const;
    Orientation orientation() const;
    OperandType operandType(OperandID operand) const;
    QString variableName(OperandID operand) const;
    qreal value(OperandID operand) const;
    QuestionType questionType() const;

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
    void swapBranches();

    Operation mOperation;
    Orientation mOrientation;
    OperandData mOperands[OperandsCount];

    QuestionType mQuestionType;
};
#endif // CMD_QUESTION_H
