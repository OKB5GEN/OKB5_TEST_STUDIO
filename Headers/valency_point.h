#ifndef VALENCY_POINT_H
#define VALENCY_POINT_H

class ShapeItem;

class ValencyPoint
{
public:
    void setPath(const QPainterPath &path);
    void setColor(const QColor &color);
    void setOwner(ShapeItem* owner);
    void setRole(int role);

    QPainterPath path() const;
    QColor color() const;
    ShapeItem* owner() const;
    int role() const;

private:
    QPainterPath mPath;
    QColor mColor;
    ShapeItem* mOwner = Q_NULLPTR;
    int mRole = 0;
};

#endif // VALENCY_POINT_H
