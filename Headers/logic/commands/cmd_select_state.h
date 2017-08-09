#ifndef CMD_SELECT_STATE_H
#define CMD_SELECT_STATE_H

#include "Headers/logic/commands/cmd_question.h"

class CmdSelectState: public CmdQuestion
{
    Q_OBJECT

public:
//    enum Orientation
//    {
//        YesDown,
//        YesRight
//    };

//    Q_ENUM(Orientation)

//    enum Operation
//    {
//        Greater,        // >
//        Less,           // <
//        Equal,          // ==
//        GreaterOrEqual, // >=
//        LessOrEqual,    // <=
//        NotEqual        // !=
//    };

//    Q_ENUM(Operation)

//    enum OperandID
//    {
//        Left,
//        Right,

//        OperandsCount
//    };

//    Q_ENUM(OperandID)

//    enum OperandType
//    {
//        Variable,
//        Number,

//        OperandNotSet
//    };

//    Q_ENUM(OperandType)

//    struct OperandData // TODO replace by QVariant
//    {
//        OperandData();

//        QString variable;
//        qreal value;
//        OperandType type;
//    };

    CmdSelectState(QObject* parent);

//    void run() override;

//    void setData(Operation operation, Orientation orientation, const OperandData& left, const OperandData& right);

//    void setOperation(Operation operation);
//    void setOrientation(Orientation orientation);
//    void setOperand(OperandID operand, qreal value);
//    void setOperand(OperandID operand, const QString& variable);

//    Operation operation() const;
//    Orientation orientation() const;
//    OperandType operandType(OperandID operand) const;
//    QString variableName(OperandID operand) const;
//    qreal value(OperandID operand) const;

protected:
//    void writeCustomAttributes(QXmlStreamWriter* writer) override;
//    void readCustomAttributes(QXmlStreamReader* reader, const AppVersion& fileVersion) override;
    void insertCommand(Command* newCmd, ValencyPoint::Role role) override;
//    bool loadFromImpl(Command* other) override;

private slots:
//    void onNameChanged(const QString& newName, const QString& oldName) override;
//    void onVariableRemoved(const QString& name) override;
//    void execute();

private:
//    void updateText() override;

//    void insertInCycle(Command* newCmd, ValencyPoint::Role role);
//    void insertInIf(Command* newCmd, ValencyPoint::Role role);
    void insertInSwitchState(Command* newCmd, ValencyPoint::Role role);

//    void insertCycleToCycle(Command* newCmd, ValencyPoint::Role role);
//    void insertCycleToIf(Command* newCmd, ValencyPoint::Role role);
    void insertCycleToSwitchState(Command* newCmd, ValencyPoint::Role role);

//    void insertIfToCycle(Command* newCmd, ValencyPoint::Role role);
//    void insertIfToIf(Command* newCmd, ValencyPoint::Role role);
    void insertIfToSwitchState(Command* newCmd, ValencyPoint::Role role);

//    void insertSwitchStateToCycle(Command* newCmd, ValencyPoint::Role role);
//    void insertSwitchStateToIf(Command* newCmd, ValencyPoint::Role role);
    void insertSwitchStateToSwitchState(Command* newCmd, ValencyPoint::Role role);

//    Operation mOperation;
//    Orientation mOrientation;
//    OperandData mOperands[OperandsCount];
};
#endif // CMD_QUESTION_H
