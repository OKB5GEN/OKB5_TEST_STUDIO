#ifndef SHAPEITEM_H
#define SHAPEITEM_H

#include <QColor>
#include <QPainterPath>
#include <QPoint>

class ShapeItem
{
public:

    void setType(int type);
    void setPath(const QPainterPath &path);
    void setToolTip(const QString &toolTip);
    void setPosition(const QPoint &position);
    void setColor(const QColor &color);

    QPainterPath path() const;
    QPoint position() const;
    QColor color() const;
    QString toolTip() const;
    int type() const;

private:
    QPainterPath myPath;
    QPoint myPosition;
    QColor myColor;
    QString myToolTip;

    int mType;
};


#endif
