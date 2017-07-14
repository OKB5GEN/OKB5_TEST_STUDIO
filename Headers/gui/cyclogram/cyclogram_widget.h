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

class CyclogramWidget : public QWidget
{
    Q_OBJECT

public:
    CyclogramWidget(QWidget* parent);
    ~CyclogramWidget();

    void clear(bool onDestroy = false);
    void setMainWindow(QWidget* widget);
    void setParentScrollArea(QScrollArea* scroll);

    ShapeItem* selectedItem() const;

    static QString delimiter();

public slots:
    void load(QSharedPointer<Cyclogram> cyclogram);
    void showValidationError(Command* cmd);
    void deleteSelectedItem();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onCyclogramStateChanged(int state);
    void removeShape(Command* command);
    void onNeedUpdate();
    void onNeedToDelete(ShapeItem* shape);
    void showSubprogramWidget();
    //void showSubprogramChart(const QStringList& variables);
    void onAppSettingsChanged();

private:
    void setSelectedItem(ShapeItem* item);

    void drawSilhouette();
    void clearSelection(bool needUpdate = true);
    QString updateWindowTitle(QWidget* dialog);
    void updateScale(const QPoint& cursorPos, int numSteps);

    void drawCyclogram(ShapeItem* item);
    void drawChildren(ShapeItem* item, const QList<Command*>& stopDrawingCommands, bool drawGoToBranch);

    ShapeItem* addShape(Command* cmd, const QPoint& cell, ShapeItem* parentShape);
    ShapeItem* addCommand(DRAKON::IconType type, const ValencyPoint& point, int param = -1);
    bool canBeDeleted(ShapeItem* item, QString& error) const;
    void deleteCommand(ShapeItem* item);
    void deleteBranch(ShapeItem* item);

    const ShapeItem* findBranch(const Command* command) const;

    int commandAt(const QPoint &pos);
    bool hasValencyPointAt(const QPoint &pos, ValencyPoint& point);

    void drawItems(QList<ShapeItem*>& items, QPainter& painter);

    void moveItemTo(const QPoint &pos);

    QString generateBranchName() const;

    ShapeItem* findExpandedItem(ShapeItem* newItem) const;
    ShapeItem* findNextBranch(const QPoint& cell) const;
    ShapeItem* addNewBranch(ShapeItem* item);

    void showEditDialog(Command* command);
    void onClickVP(const ValencyPoint& point);
    void showContextMenuForVP(const ValencyPoint& point, const QPoint& pos);

    QList<ShapeItem*> mCommands;
    ShapeItem* mSihlouetteArrow;
    ShapeItem* mSihlouetteLine;

    QPoint mPreviousPosition;
    ShapeItem * mMovingItem;
    ShapeItem * mSelectedItem;

    ShapeItem* mRootShape;

    QWeakPointer<Cyclogram> mCurrentCyclogram;

    CmdSubProgram* mCurSubprogram;

    QWidget* mMainWindow;

    qreal mScale;
    QScrollArea* mParentScrollArea;

signals:
    void selectionChanged(ShapeItem* item);
};

#endif //CYCLOGRAM_WIDGET_H
