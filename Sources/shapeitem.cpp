#include <QtWidgets>

#include "Headers/shapeitem.h"

QPainterPath ShapeItem::path() const
{
    //QPainterPath p = mPath;
    //p.addPath(mTextPath);
    //return p;
    return mPath;
}

QPainterPath ShapeItem::textPath() const
{
    //QPainterPath p = mPath;
    //p.addPath(mTextPath);
    //return p;
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

QPoint ShapeItem::cell() const
{
    return mCell;
}

void ShapeItem::setCell(const QPoint &position)
{
    mCell = position;
}

void ShapeItem::setText(const QString& text)
{
    mText = text;
}

const QString& ShapeItem::text() const
{
    return mText;
}
