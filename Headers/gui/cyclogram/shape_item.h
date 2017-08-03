#ifndef SHAPE_ITEM_H
#define SHAPE_ITEM_H

#include <QColor>
#include <QPainterPath>
#include <QPoint>
#include <QFont>

class Command;
class ValencyPoint;

class ShapeItem: public QObject
{
    Q_OBJECT

public:
    ShapeItem(QObject* parent);
    ~ShapeItem();

    void setPath(const QPainterPath &path);
    void setToolTip(const QString &toolTip);
    void setColor(const QColor &color);
    void setCell(const QPoint &cell);
    void setCommand(Command* command);
    void setRect(const QRect& rect, bool pushToChildren);
    void setSelected(bool selected);
    void setParentShape(ShapeItem* parent);

    void setPosition(const QPoint& position);

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
    const QStringList& multilineText() const;
    QPoint cell() const;
    Command* command() const;
    const QList<ValencyPoint*>& valencyPoints() const;
    ValencyPoint* valencyPoint(int role) const;
    QRect rect() const;
    ShapeItem* parentShape() const;
    ShapeItem* childShape(int index) const;
    const QList<ShapeItem*>& childShapes() const;

    static const QSizeF& itemSize(bool needUpdate = false);
    static const QSizeF& cellSize(bool needUpdate = false);
    static const QPointF& origin(bool needUpdate = false);

    void pushDown();
    void pullUp();

    void onChildRectChanged(ShapeItem * shape);

public slots:
    void onAppSettingsChanged();

signals:
    void changed();
    void needToDelete(ShapeItem* shape);

private slots:
    void onTextChanged(const QString& text);
    void onErrorStatusChanged(bool hasErrors);
    void setActive(bool active);

private:
    void updateMulilineText();
    void createPath();
    void createValencyPoints(Command* cmd);

    void removeQuestionBranch(ShapeItem* branch);
    ShapeItem* findShape(Command* cmd, int& role);

    void updateCyclogramRect(ShapeItem* changedBranch);
    bool canSetRect(const QRect& rect) const;
    int minHeight() const;

    void updateToolTip();

    QPainterPath mPath;             // shape path
    QPainterPath mTextPath;         // optional path
    QPainterPath mAdditionalPath;   // path for some not-interactive display (arrow line)
    QPainterPath mArrowPath;        // path for arrow drawing (arrow triangle)
    QPoint mPosition;               // top-left corner of the shape in window coordinates
    QPoint mCell;                   // cell, occupied by command shape itself
    QRect mRect;                    // bounding rect of the command (need for drawing command connections and QUESTION arrow)
    QColor mColor;                  // color for filling shape background
    QColor mAdditionalColor;        // color for filling additional path background (hack for connection lines coloring)
    QColor mActiveColor;            // color for filling shape background in command active state (being executed)
    QString mToolTip;               // tooltip text for shape
    QList<ValencyPoint*> mValencyPoints; // valency points list for this command

    bool mActive;

    Command* mCommand; // data pointer for the command logics
    ShapeItem* mParentShape;

    QList<ShapeItem*> mChildShapes;

    QStringList mMultilineText;
};

#endif //SHAPE_ITEM_H
