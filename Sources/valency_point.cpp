#include <QtWidgets>

#include "Headers/valency_point.h"

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

void ValencyPoint::setRole(int role)
{
    mRole = role;
}

int ValencyPoint::role() const
{
    return mRole;
}
