#ifndef SHAPEITEM_H
#define SHAPEITEM_H

#include <QColor>
#include <QPainterPath>
#include <QPoint>

#include "Headers/valency_point.h"

class Command;

class ShapeItem
{
public:
    enum ShapeFlags
    {
        Selectable  = 0x00000001,
        Movable     = 0x00000002,
        Editable    = 0x00000004
    };

    void setPath(const QPainterPath &path);
    void setTextPath(const QPainterPath &path);
    void setToolTip(const QString &toolTip);
    void setPosition(const QPoint &position);
    void setColor(const QColor &color);
    void setFlags(uint32_t flags);
    void setCell(const QPoint &position);
    void setCommand(Command* command);
    void setValencyPoints(const QList<ValencyPoint>& points);
    void setRect(const QRect& rect);

    QPainterPath path() const;
    QPainterPath textPath() const;
    QPoint position() const;
    QColor color() const;
    QString toolTip() const;
    uint32_t flags() const;
    QPoint cell() const;
    Command* command() const;
    const QList<ValencyPoint>& valencyPoints() const;
    QRect rect() const;

private:
    QPainterPath mPath; // shape path
    QPainterPath mTextPath; // path for text iside shape
    QPoint mPosition; // top-left corner of the shape in window coordinates
    QPoint mCell; // cell, occupied by command shape itself
    QRect mRect; // bounding rect of the command (need for drawing command connections and QUESTION arrow)
    QColor mColor; // color for filling shape background
    QString mToolTip; // tooltip text for shape
    QList<ValencyPoint> mValencyPoints; // valency points list for this command

    uint32_t mFlags = 0; // ShapeFlags here, by default the shape is not interactive

    Command* mCommand = Q_NULLPTR; // data pointer for the command logics
};

#endif
