#include <QtWidgets>

#include "Headers/shapeitem.h"

QPainterPath ShapeItem::path() const
{
    return mPath;
}

QPainterPath ShapeItem::textPath() const
{
    return mTextPath;
}

QPoint ShapeItem::position() const
{
    return mPosition;
}

QColor ShapeItem::color() const
{
    return mColor;
}

QString ShapeItem::toolTip() const
{
    return mToolTip;
}

void ShapeItem::setPath(const QPainterPath &path)
{
    mPath = path;
}

void ShapeItem::setTextPath(const QPainterPath &path)
{
    mTextPath = path;
}

void ShapeItem::setToolTip(const QString &toolTip)
{
    mToolTip = toolTip;
}

void ShapeItem::setPosition(const QPoint &position)
{
    mPosition = position;
}

void ShapeItem::setColor(const QColor &color)
{
    mColor = color;
}

void ShapeItem::setFlags(uint32_t flags)
{
    mFlags = flags;
}

uint32_t ShapeItem::flags() const
{
    return mFlags;
}

QPoint ShapeItem::cell() const
{
    return mCell;
}

void ShapeItem::setCell(const QPoint &position)
{
    mCell = position;
}

void ShapeItem::setCommand(Command* command)
{
    mCommand = command;
}

Command* ShapeItem::command() const
{
    return mCommand;
}

const QList<ValencyPoint>& ShapeItem::valencyPoints() const
{
    return mValencyPoints;
}

void ShapeItem::setValencyPoints(const QList<ValencyPoint>& points)
{
    mValencyPoints = points;
    for (int i = 0, sz = mValencyPoints.size(); i < sz; ++i)
    {
        mValencyPoints[i].setOwner(this);
    }
}

void ShapeItem::setRect(const QRect& rect)
{
    mRect = rect;
}

QRect ShapeItem::rect() const
{
    return mRect;
}
