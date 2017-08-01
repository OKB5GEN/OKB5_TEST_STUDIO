#ifndef VALENCY_POINT_H
#define VALENCY_POINT_H

#include <QPainterPath>
#include <QColor>
#include <QSet>

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

    static QPainterPath createPath();
    static QColor allowedColor();
    static QColor forbiddenColor();

    void setOwner(ShapeItem* owner);
    void setPath(const QPainterPath &path);
    void setColor(const QColor &color);
    void setRole(Role role);

    void setCurrentCommandType(int type);
    void setPressed(bool pressed);

    void removeInsertableCommand(int commandID);
    void addInsertableCommand(int commandID);
    void setInsertableCommand(int commandID);
    void setInsertableCommands(const QSet<int>& commands);
    const QSet<int>& insertableCommands() const;
    bool canBeInserted(int commandID) const;

    QPainterPath path() const;
    QColor color() const;
    ShapeItem* owner() const;
    Role role() const;

private:
    ShapeItem* mOwner = Q_NULLPTR;
    QPainterPath mPath;
    QColor mColor;
    Role mRole;

    QSet<int> mInsertableCommands;
};

#endif // VALENCY_POINT_H
