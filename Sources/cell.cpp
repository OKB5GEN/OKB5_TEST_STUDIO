#include "Headers/cell.h"

Cell::Cell():
    mPos(QPoint(-1, -1)),
    mType(EMPTY),
    mCommand(Q_NULLPTR)
{
}

Cell::Cell(QPoint pos, Command* cmd):
    mPos(pos),
    mType(COMMAND),
    mCommand(cmd)
{
}

Cell::Cell(QPoint pos, Type type):
    mPos(pos),
    mType(type),
    mCommand(Q_NULLPTR)
{
}

const QPoint& Cell::pos() const
{
    return mPos;
}

void Cell::setPos(const QPoint& pos)
{
    mPos = pos;
}

Cell::Type Cell::type() const
{
    return mType;
}

void Cell::setType(Type type)
{
    mType = type;
}

Command* Cell::command() const
{
    return mCommand;
}

void Cell::setCommand(Command* command)
{
    mCommand = command;
}

const QString& Cell::text() const
{
    return mText;
}

void Cell::setText(const QString& text)
{
    mText = text;
}
