#include <QtWidgets>

#include "Headers/shapeitem.h"

QPainterPath ShapeItem::path() const
{
    return myPath;
}

QPoint ShapeItem::position() const
{
    return myPosition;
}

QColor ShapeItem::color() const
{
    return myColor;
}

QString ShapeItem::toolTip() const
{
    return myToolTip;
}

void ShapeItem::setPath(const QPainterPath &path)
{
    myPath = path;
}

void ShapeItem::setToolTip(const QString &toolTip)
{
    myToolTip = toolTip;
}

void ShapeItem::setPosition(const QPoint &position)
{
    myPosition = position;
}

void ShapeItem::setColor(const QColor &color)
{
    myColor = color;
}

void ShapeItem::setType(ShapeTypes type)
{
    mType = type;
}

ShapeTypes ShapeItem::type() const
{
    return mType;
}

void ShapeItem::setFlags(uint32_t flags)
{
    mFlags = flags;
}

uint32_t ShapeItem::flags() const
{
    return mFlags;
}

