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
    mType(type)
{
}
