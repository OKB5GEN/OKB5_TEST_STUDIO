#ifndef SORTINGBOX_H
#define SORTINGBOX_H

#include <QWidget>

#include "Headers/shapetypes.h"
#include "Headers/shapeitem.h"

QT_BEGIN_NAMESPACE
class QAction;
class QPoint;
class QToolButton;
class QDialog;
QT_END_NAMESPACE

class ShapeAddDialog;
class ShapeEditDialog;
class Cyclogram;
class Command;

class SortingBox : public QWidget
{
    Q_OBJECT

public:
    SortingBox();
    ~SortingBox();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

public slots:
    void load(Cyclogram* cyclogram);

private:
    void clear();
    void drawSilhouette();

    //int updateButtonGeometry(QToolButton *button, int x, int y);

    ShapeItem* createCommandShape(Command* cmd, const QPoint& cell);
    ValencyPoint createPoint(const QPointF& point);
    bool isCyclogramEndBranch(Command* cmd) const;
    void addCommand(ShapeTypes type, const ValencyPoint& point);

    int commandAt(const QPoint &pos);
    bool hasValencyPointAt(const QPoint &pos, ValencyPoint& point);

    void drawItems(QList<ShapeItem*>& items, QPainter& painter);

    QPainterPath createPath(ShapeItem* item) const;
    QList<ValencyPoint> createValencyPoints(Command* cmd);

    void moveItemTo(const QPoint &pos);

    void addText(ShapeItem* item);
    void addChildCommands(Command* parentCmd, const QPoint& parentCell);
    bool isBranchExist(Command* goToBranchCmd);

    ShapeItem* findExpandedItem(ShapeItem* newItem) const;
    void updateItemGeometry(ShapeItem* item, int xShift, int yShift, int topShift, int bottomShift) const;

    QPointF mOrigin;
    QSizeF mItem;
    QSize mDiagramSize;

    QList<ShapeItem*> mCommands;
    QList<ShapeItem*> mSihlouette; // sihlouette (TODO temporary)

    QPoint mPreviousPosition;
    ShapeItem * mSelectedItem;

    QFont mFont;

    ShapeAddDialog * mShapeAddDialog;
    ShapeEditDialog * mShapeEditDialog;

    Cyclogram* mCurrentCyclogram = Q_NULLPTR;
};

#endif
