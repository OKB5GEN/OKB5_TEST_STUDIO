#ifndef VALENCY_POINT_H
#define VALENCY_POINT_H

#include <QPainterPath>
#include <QColor>

class ShapeItem;

class ValencyPoint
{
public:
    enum Role // roles of valency point
    {
        Down        = 0, // Actual for all commands
        Right       = 1, // Actual for BRANCH_BEGIN and QUESTION
        UnderArrow  = 2  // Actual for QUESTION
    };

    ValencyPoint();

    void setPath(const QPainterPath &path);
    void setColor(const QColor &color);
    void setOwner(ShapeItem* owner);
    void setRole(Role role);

    QPainterPath path() const;
    QColor color() const;
    ShapeItem* owner() const;
    Role role() const;

private:
    QPainterPath mPath;
    QColor mColor;
    ShapeItem* mOwner = Q_NULLPTR;
    Role mRole;
};

#endif // VALENCY_POINT_H
