#ifndef SHAPEITEM_H
#define SHAPEITEM_H

#include <QColor>
#include <QPainterPath>
#include <QPoint>

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

    QPainterPath path() const;
    QPainterPath textPath() const;
    QPoint position() const;
    QColor color() const;
    QString toolTip() const;
    uint32_t flags() const;
    QPoint cell() const;
    Command* command() const;

private:
    QPainterPath mPath;
    QPainterPath mTextPath;
    QPoint mPosition;
    QColor mColor;
    QString mToolTip;

    uint32_t mFlags;
    QPoint mCell;

    Command* mCommand = Q_NULLPTR;
};

#endif
