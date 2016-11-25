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

/*

QPainterPath ValencyPoint::textPath() const
{
    return mTextPath;
}

QPoint ValencyPoint::position() const
{
    return mPosition;
}

QString ValencyPoint::toolTip() const
{
    return mToolTip;
}

void ValencyPoint::setTextPath(const QPainterPath &path)
{
    mTextPath = path;
}

void ValencyPoint::setToolTip(const QString &toolTip)
{
    mToolTip = toolTip;
}

void ValencyPoint::setPosition(const QPoint &position)
{
    mPosition = position;
}

void ValencyPoint::setFlags(uint32_t flags)
{
    mFlags = flags;
}

uint32_t ValencyPoint::flags() const
{
    return mFlags;
}

QPoint ValencyPoint::cell() const
{
    return mCell;
}

void ValencyPoint::setCell(const QPoint &position)
{
    mCell = position;
}

void ValencyPoint::setCommand(Command* command)
{
    mCommand = command;
}

Command* ValencyPoint::command() const
{
    return mCommand;
}
*/
