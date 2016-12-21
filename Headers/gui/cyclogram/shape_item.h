#ifndef SHAPE_ITEM_H
#define SHAPE_ITEM_H

#include <QColor>
#include <QPainterPath>
#include <QPoint>
#include <QFont>

#include "Headers/gui/cyclogram/valency_point.h"

class Command;

class ShapeItem: public QObject
{
    Q_OBJECT

public:
    ShapeItem(QObject* parent);

    void setPath(const QPainterPath &path);
    void setTextPath(const QPainterPath &path);
    void setToolTip(const QString &toolTip);
    void setColor(const QColor &color);
    void setCell(const QPoint &cell);
    void setCommand(Command* command);
    void setRect(const QRect& rect, bool pushToChildren);
    void setSelected(bool selected);
    void setParentShape(ShapeItem* parent);

    void setChildShape(ShapeItem* item, int index);
    void addChildShape(ShapeItem* item);
    void removeChildShape(ShapeItem* item);
    void replaceChildShape(ShapeItem* newItem, ShapeItem* oldItem);

    void remove();
    void adjust();

    QPainterPath path() const;
    QPainterPath textPath() const;
    QPainterPath additionalPath() const;
    QPainterPath arrowPath() const;
    QPoint position() const;
    QColor color() const;
    QColor additionalColor() const;
    QString toolTip() const;
    QPoint cell() const;
    Command* command() const;
    const QList<ValencyPoint>& valencyPoints() const;
    ValencyPoint valencyPoint(int role) const;
    QRect rect() const;
    ShapeItem* parentShape() const;
    ShapeItem* childShape(int index) const;

    static const QSizeF& itemSize();
    static const QSizeF& cellSize();
    static const QPointF& origin();

    // TODO remove >>>
    void pushDown();
    void pullUp();
    //<<<

    void onChildRectChanged(ShapeItem * shape);

signals:
    void changed();

private slots:
    void onTextChanged(const QString& text);
    void onErrorStatusChanged(bool status);
    void setActive(bool active);

private:
    void createPath();
    void setValencyPoints(const QList<ValencyPoint>& points);
    ValencyPoint createValencyPoint(const QPointF& point, ValencyPoint::Role role);
    void createValencyPoints(Command* cmd);
    bool isCyclogramEndBranch(Command* cmd) const;

    void updateCyclogramRect(ShapeItem* changedBranch);
    bool canSetRect(const QRect& rect) const;
    int minHeight() const;

    QPainterPath mPath;             // shape path
    QPainterPath mTextPath;         // path for text iside shape
    QPainterPath mAdditionalPath;   // path for some not-interactive display (arrow line)
    QPainterPath mArrowPath;        // path for arrow drawing (arrow triangle)
    QPoint mPosition;               // top-left corner of the shape in window coordinates
    QPoint mCell;                   // cell, occupied by command shape itself
    QRect mRect;                    // bounding rect of the command (need for drawing command connections and QUESTION arrow)
    QColor mColor;                  // color for filling shape background
    QColor mAdditionalColor;        // color for filling additional path background (hack for connection lines coloring)
    QColor mActiveColor;            // color for filling shape background in command active state (being executed)
    QString mToolTip;               // tooltip text for shape
    QList<ValencyPoint> mValencyPoints; // valency points list for this command

    QFont mFont; // font for writing texts

    bool mActive;

    Command* mCommand = Q_NULLPTR; // data pointer for the command logics
    ShapeItem* mParentShape = Q_NULLPTR;

    QList<ShapeItem*> mChildShapes;
};

#endif //SHAPE_ITEM_H
