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
    void insertItem(ShapeTypes id, const QPoint& pos, const QString& text, int shapeAddItemIndex);

    void connectItems(const QPoint& pos1, const QPoint& pos2, int addItemCount);
    void drawSilhouette();

    //int updateButtonGeometry(QToolButton *button, int x, int y);

    void createCommandShape(Command* cmd, const QPoint& cell);
    void createConnectorShape(const QPainterPath &path, const QPoint &pos);
    void createValencyPointShape(const QPoint &pos, const QPoint& cell);

    int itemAt(const QPoint &pos, const QList<ShapeItem>& items);

    void drawItems(QList<ShapeItem>& items, QPainter& painter);

    QPainterPath createPath(ShapeTypes type);

    void moveItemTo(const QPoint &pos);

    void addText(ShapeItem& item);
    void addChildCommands(Command* parentCmd, const QPoint& parentCell);
    bool isHeadlineExist(Command* parentCmd);

    QPointF mOrigin;
    QSizeF mItem;
    QSize mDiagramSize;

    QList<ShapeItem> mCommands; // cyclogram commands (created by user, editable)
    QList<ShapeItem> mConnectors; // connectors between cyclogram commands (created by program, not editable)
    QList<ShapeItem> mValencyPoints; // points where user can add new cyclogram commands
    QList<ShapeItem> mSihlouette; // sihlouette (TODO temporary)

    QPoint mPreviousPosition;
    ShapeItem * mSelectedItem;

    QFont mFont;

    ShapeAddDialog * mShapeAddDialog;
    ShapeEditDialog * mShapeEditDialog;
};

#endif
