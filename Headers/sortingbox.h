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
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

public slots:
    void load(Cyclogram* cyclogram);

private slots:
    void onCyclogramStateChanged(int state);

private:
    void clear(bool onDestroy = false);
    void drawSilhouette();
    void clearSelection(bool needUpdate = true);

    ShapeItem* createCommandShape(Command* cmd, const QPoint& cell);
    ValencyPoint createPoint(const QPointF& point, int role);
    bool isCyclogramEndBranch(Command* cmd) const;
    ShapeItem* addCommand(DRAKON::IconType type, const ValencyPoint& point);
    void deleteCommand(ShapeItem* item);

    int commandAt(const QPoint &pos);
    bool hasValencyPointAt(const QPoint &pos, ValencyPoint& point);

    void drawItems(QList<ShapeItem*>& items, QPainter& painter);

    QList<ValencyPoint> createValencyPoints(Command* cmd);

    void moveItemTo(const QPoint &pos);

    void addChildCommands(Command* parentCmd, const QPoint& parentCell);
    bool isBranchExist(Command* goToBranchCmd);
    QString generateBranchName() const;

    ShapeItem* findExpandedItem(ShapeItem* newItem) const;
    ShapeItem* findNextBranch(const QPoint& cell) const;
    ShapeItem* addNewBranch(ShapeItem* item);
    void updateItemGeometry(ShapeItem* item, int xShift, int yShift, int topShift, int bottomShift) const;

    void showEditDialog(ShapeItem* item);

    QPointF mOrigin;
    QSize mDiagramSize;

    QList<ShapeItem*> mCommands;
    QList<ShapeItem*> mSihlouette; // sihlouette (TODO temporary)

    QPoint mPreviousPosition;
    ShapeItem * mMovingItem;
    ShapeItem * mSelectedItem;

    // TODO move dialogs to main window
    ShapeAddDialog * mShapeAddDialog;
    ShapeEditDialog * mShapeEditDialog;

    QMap<DRAKON::IconType, QDialog*> mEditDialogs;

    Cyclogram* mCurrentCyclogram = Q_NULLPTR;
};

#endif
