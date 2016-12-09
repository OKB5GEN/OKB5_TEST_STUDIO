#ifndef CYCLOGRAM_WIDGET_H
#define CYCLOGRAM_WIDGET_H

#include <QWidget>
#include <QMap>

#include "Headers/shape_types.h"
#include "Headers/shape_item.h"

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

class CyclogramWidget : public QWidget
{
    Q_OBJECT

public:
    CyclogramWidget(QWidget* parent);
    ~CyclogramWidget();

public slots:
    void load(Cyclogram* cyclogram);
    void showValidationError(Command* cmd);

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onCyclogramStateChanged(int state);
    void removeShape(Command* command);
    void onNeedUpdate();

private:
    void clear(bool onDestroy = false);
    void drawSilhouette();
    void clearSelection(bool needUpdate = true);

    ShapeItem* createCommandShape(Command* cmd, const QPoint& cell);
    ValencyPoint createPoint(const QPointF& point, int role);
    bool isCyclogramEndBranch(Command* cmd) const;
    ShapeItem* addCommand(DRAKON::IconType type, const ValencyPoint& point);

    bool canBeDeleted(ShapeItem* item, QString& error) const;
    void deleteCommand(ShapeItem* item);
    void deleteBranch(ShapeItem* item);

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

    void showEditDialog(Command* command);

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

#endif //CYCLOGRAM_WIDGET_H
