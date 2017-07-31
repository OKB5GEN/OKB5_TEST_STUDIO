#include <QtWidgets>

#include "Headers/gui/cyclogram/valency_point.h"
#include "Headers/gui/cyclogram/shape_item.h"

ValencyPoint::ValencyPoint() :
    mOwner(Q_NULLPTR),
    mRole(Down),
    mCanBeLanded(false)
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

void ValencyPoint::setCanBeLanded(bool canBeLanded)
{
    mCanBeLanded = canBeLanded;
}

bool ValencyPoint::canBeLanded() const
{
    return mCanBeLanded;
}

QPainterPath ValencyPoint::createPath()
{
    QPainterPath path;

    qreal crossSize = 0.6;
    qreal radius = qMin(ShapeItem::cellSize().width(), ShapeItem::cellSize().height()) / 3;
    path.addEllipse(QRectF(-radius, -radius, radius * 2, radius * 2));
    path.moveTo(0, -radius * crossSize);
    path.lineTo(0, radius * crossSize);
    path.moveTo(-radius * crossSize, 0);
    path.lineTo(radius * crossSize, 0);

    return path;
}
