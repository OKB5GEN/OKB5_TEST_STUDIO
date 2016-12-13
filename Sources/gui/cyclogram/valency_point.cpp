#include <QtWidgets>

#include "Headers/gui/cyclogram/valency_point.h"

ValencyPoint::ValencyPoint() :
    mOwner(Q_NULLPTR),
    mRole(Down)
{
}

QPainterPath ValencyPoint::path() const
{
    return mPath;
}

void ValencyPoint::setPath(const QPainterPath &path)
{
    mPath = path;
}

void ValencyPoint::setColor(const QColor &color)
{
    mColor = color;
}

QColor ValencyPoint::color() const
{
    return mColor;
}

void ValencyPoint::setOwner(ShapeItem* owner)
{
    mOwner = owner;
}

ShapeItem* ValencyPoint::owner() const
{
    return mOwner;
}

void ValencyPoint::setRole(ValencyPoint::Role role)
{
    mRole = role;
}

ValencyPoint::Role ValencyPoint::role() const
{
    return mRole;
}
