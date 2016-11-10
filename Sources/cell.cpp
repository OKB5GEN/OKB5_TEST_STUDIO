#include "Headers/cell.h"

Cell::Cell():
    mPos(QPoint(-1, -1))
{
}

Cell::Cell(QPoint pos, Type type):
    mPos(pos),
    mType(type)
{

}
