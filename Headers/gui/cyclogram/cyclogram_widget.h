#ifndef CYCLOGRAM_WIDGET_H
#define CYCLOGRAM_WIDGET_H

#include <QWidget>
#include <QMap>

#include "Headers/shape_types.h"
#include "Headers/gui/cyclogram/valency_point.h"

class QAction;
class QPoint;
class QToolButton;
class QDialog;
class QScrollArea;

class Cyclogram;
class Command;
class ShapeItem;
class CmdSubProgram;
class Clipboard;

class CyclogramWidget : public QWidget
{
    Q_OBJECT

public:
    CyclogramWidget(QWidget* parent);
    ~CyclogramWidget();

    void clear(bool onDestroy = false);
    void setMainWindow(QWidget* widget);
    void setParentScrollArea(QScrollArea* scroll);
    void setClipboard(QSharedPointer<Clipboard> clipboard);

    ShapeItem* selectedItem() const;

    static QString delimiter();

public slots:
    void load(QSharedPointer<Cyclogram> cyclogram);
    void showValidationError(Command* cmd);
    void deleteSelectedItem();
    void setCurrentCommandType(int command);
    void clearSelection(bool needUpdate = true);

protected:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onCyclogramStateChanged(int state);
    void removeShape(Command* command);
    void onNeedUpdate();
    void onNeedToDelete(ShapeItem* shape);
    void showSubprogramWidget(CmdSubProgram* subprogram);
    void onAppSettingsChanged();

private:
    static ShapeItem* createShape(Command* cmd, const QPoint& cell, ShapeItem* parentShape, QObject* parent);

    void drawCommandText(ShapeItem* item, QPainter& painter);
    void updateCursor(const QPoint& pos);
    void setSelectedItem(ShapeItem* item);

    void drawSilhouette();
    QString updateWindowTitle(QWidget* dialog, CmdSubProgram* subprogram);
    void updateScale(const QPoint& cursorPos, int numSteps);

    void drawCyclogram(ShapeItem* item);
    void drawChildren(ShapeItem* item, const QList<Command*>& stopDrawingCommands, bool drawGoToBranch);

    ShapeItem* addShape(Command* cmd, const QPoint& cell, ShapeItem* parentShape);
    ShapeItem* addNewCommand(DRAKON::IconType type, const ValencyPoint* point);
    ShapeItem* addCommand(Command* cmd, const ValencyPoint* point);
    bool canBeDeleted(ShapeItem* item, QString& error) const;
    bool canBeMoved(ShapeItem* item, const ValencyPoint* point) const;
    void deleteCommand(ShapeItem* item);
    void deleteBranch(ShapeItem* item);

    void onDragStart();
    void onDragFinish();

    const ShapeItem* findBranch(const Command* command) const;

    void copyCommandTo(Command* commandToCopy, const ValencyPoint* point);
    void copyBranchTo(Command* commandToCopy, const ValencyPoint* point);
    void moveBranchTo(ShapeItem* branchToMove, const ValencyPoint* point);
    ShapeItem* copyBranch(Command* branchCmd, const QPoint& cell);

    ShapeItem* shapeAt(const QPoint &pos) const;
    ValencyPoint* valencyPointAt(const QPoint &pos) const;

    void drawItems(QList<ShapeItem*>& items, QPainter& painter);

    void moveItemTo(const QPoint &pos);

    QPoint calculateNewCommandCell(const ValencyPoint* point);
    void updateWidgetShapes(ShapeItem* newShape, const ValencyPoint* point);

    ShapeItem* findExpandedItem(ShapeItem* newItem) const;
    ShapeItem* findNextBranch(const QPoint& cell) const;
    ShapeItem* addNewBranch(ShapeItem* item);

    void showEditDialog(Command* command);
    void onClickVP(const ValencyPoint* point, const QPoint& pos);
    void showContextMenuForVP(const ValencyPoint* point, const QPoint& pos);
    void showContextMenuForCommand(ShapeItem* item, const QPoint& pos);

    int mMouseButtonState;

    QList<ShapeItem*> mShapes;
    ShapeItem* mSihlouetteArrow;
    ShapeItem* mSihlouetteLine;

    QPoint mDragStartPosition;
    QPoint mPreviousPosition;
    ShapeItem* mDraggingShape;
    ShapeItem* mSelectedShape;
    ShapeItem* mPressedShape;

    ValencyPoint* mPressedVP;

    ShapeItem* mRootShape;

    QWeakPointer<Cyclogram> mCyclogram;

    QWidget* mMainWindow;

    qreal mScale;
    QScrollArea* mParentScrollArea;
    qreal mScrollSpeed;

    int mCurrentCommandType;
    QFont mFont;

    QWeakPointer<Clipboard> mClipboard;

signals:
    void selectionChanged(ShapeItem* item);
};

#endif //CYCLOGRAM_WIDGET_H
