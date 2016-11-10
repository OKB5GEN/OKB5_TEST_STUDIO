#ifndef CELL_H
#define CELL_H

#include <QPoint>

class Command;

class Cell
{
public:
    enum Type
    {
        EMPTY,      // no any operation performed, only connector will be drawn
        ADD_COMMAND,// command can be inserted in this cell
        COMMAND     // command must be executed in this cell
    };

    Cell();
    Cell(QPoint pos, Type type);

    const QPoint& getPos() const { return mPos; }
    void setPos(const QPoint& pos) { mPos = pos; }

    Type getType() const { return mType; }
    void setType(Type type) { mType = type; }

    Command* getCommand() const { return mCommand; }
    void setCommand(Command* command) { mCommand = command; }

private:
    QPoint mPos;
    Type mType = EMPTY;
    Command * mCommand = Q_NULLPTR;
};
#endif // CELL_H
