#ifndef VALENCY_POINT_H
#define VALENCY_POINT_H

class ShapeItem;

class ValencyPoint
{
public:
    void setPath(const QPainterPath &path);
    void setColor(const QColor &color);
    void setOwner(ShapeItem* owner);

    QPainterPath path() const;
    QColor color() const;
    ShapeItem* owner() const;

private:
    QPainterPath mPath;
    QColor mColor;
    ShapeItem* mOwner = Q_NULLPTR;
};

#endif // VALENCY_POINT_H
