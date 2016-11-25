#ifndef VALENCY_POINT_H
#define VALENCY_POINT_H

/*
#include <QColor>
#include <QPainterPath>
#include <QPoint>

class Command;
*/

class ValencyPoint
{
public:
    void setPath(const QPainterPath &path);
    void setColor(const QColor &color);

    QPainterPath path() const;
    QColor color() const;

private:
    QPainterPath mPath;
    QColor mColor;

/*    enum ShapeFlags
    {
        Selectable  = 0x00000001,
        Movable     = 0x00000002,
        Editable    = 0x00000004
    };


    void setTextPath(const QPainterPath &path);
    void setToolTip(const QString &toolTip);
    void setPosition(const QPoint &position);

    void setFlags(uint32_t flags);
    void setCell(const QPoint &position);
    void setCommand(Command* command);


    QPainterPath textPath() const;
    QPoint position() const;

    QString toolTip() const;
    uint32_t flags() const;
    QPoint cell() const;
    Command* command() const;

private:

    QPainterPath mTextPath;
    QPoint mPosition;
    QString mToolTip;
    uint32_t mFlags;
    QPoint mCell;

    Command* mCommand = Q_NULLPTR;*/
};

#endif // VALENCY_POINT_H
