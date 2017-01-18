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
        UnderArrow  = 2, // Actual for QUESTION
        CaseBase    = 3  //TODO Actual for SWITCH-CASE as (CaseBase + CASE)
    };

    ValencyPoint();

    void setOwner(ShapeItem* owner);
    void setPath(const QPainterPath &path);
    void setColor(const QColor &color);
    void setRole(Role role);
    void setCanBeLanded(bool canBeLanded);

    QPainterPath path() const;
    QColor color() const;
    ShapeItem* owner() const;
    Role role() const;
    bool canBeLanded() const;

private:
    ShapeItem* mOwner = Q_NULLPTR;
    QPainterPath mPath;
    QColor mColor;
    Role mRole;
    bool mCanBeLanded;
};

#endif // VALENCY_POINT_H
