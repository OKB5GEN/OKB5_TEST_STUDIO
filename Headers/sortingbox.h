#ifndef SORTINGBOX_H
#define SORTINGBOX_H

#include <QWidget>
#include <QMap>

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

    ShapeItem* createCommandShape(Command* cmd, const QPoint& cell);
    ValencyPoint createPoint(const QPointF& point, int role);
    bool isCyclogramEndBranch(Command* cmd) const;
    void addCommand(DRAKON::IconType type, const ValencyPoint& point);

    int commandAt(const QPoint &pos);
    bool hasValencyPointAt(const QPoint &pos, ValencyPoint& point);

    void drawItems(QList<ShapeItem*>& items, QPainter& painter);

    QList<ValencyPoint> createValencyPoints(Command* cmd);

    void moveItemTo(const QPoint &pos);

    void addChildCommands(Command* parentCmd, const QPoint& parentCell);
    bool isBranchExist(Command* goToBranchCmd);

    ShapeItem* findExpandedItem(ShapeItem* newItem) const;
    void updateItemGeometry(ShapeItem* item, int xShift, int yShift, int topShift, int bottomShift) const;

    void showEditDialog(ShapeItem* item);

    QPointF mOrigin;
    QSize mDiagramSize;

    QList<ShapeItem*> mCommands;
    QList<ShapeItem*> mSihlouette; // sihlouette (TODO temporary)

    QPoint mPreviousPosition;
    ShapeItem * mSelectedItem;

    // TODO move dialogs to main window
    ShapeAddDialog * mShapeAddDialog;
    ShapeEditDialog * mShapeEditDialog;

    QMap<DRAKON::IconType, QDialog*> mEditDialogs;

    Cyclogram* mCurrentCyclogram = Q_NULLPTR;
};

#endif
