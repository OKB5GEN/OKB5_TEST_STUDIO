#ifndef SHAPEITEM_H
#define SHAPEITEM_H

#include <QColor>
#include <QPainterPath>
#include <QPoint>

#include "Headers/shapetypes.h"

class ShapeItem
{
public:
    enum ShapeFlags
    {
        Selectable  = 0x00000001,
        Movable     = 0x00000002,
        Editable    = 0x00000004
    };

    void setText(const QString& text);
    void setType(ShapeTypes type);
    void setPath(const QPainterPath &path);
    void setTextPath(const QPainterPath &path);
    void setToolTip(const QString &toolTip);
    void setPosition(const QPoint &position);
    void setColor(const QColor &color);
    void setFlags(uint32_t flags);
    void setCell(const QPoint &position);

    QPainterPath path() const;
    QPainterPath textPath() const;
    QPoint position() const;
    QColor color() const;
    QString toolTip() const;
    ShapeTypes type() const;
    uint32_t flags() const;
    QPoint cell() const;
    const QString& text() const;

private:
    QPainterPath mPath;
    QPainterPath mTextPath;
    QPoint mPosition;
    QColor mColor;
    QString mToolTip;

    ShapeTypes mType;
    uint32_t mFlags;
    QPoint mCell;
    QString mText = "Default";
};


#endif