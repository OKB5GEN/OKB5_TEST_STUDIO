#ifndef SORTINGBOX_H
#define SORTINGBOX_H

#include <QWidget>

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

private slots:
    void addItem(ShapeTypes id, const QPoint& pos, const QString& text);

private:
    void insertItem(ShapeTypes id, const QPoint& pos, const QString& text, int shapeAddItemIndex);

    void connectItems(const QPoint& pos1, const QPoint& pos2, int addItemCount);
    void drawSilhouette();

    int updateButtonGeometry(QToolButton *button, int x, int y);
    void createShapeItem(const QPainterPath &path, const QString &toolTip, const QPoint &pos, const QColor &color, ShapeTypes type, const QPoint &cell, const QString& text);
    int itemAt(const QPoint &pos);
    void moveItemTo(const QPoint &pos);
    QPoint initialItemPosition(const QPainterPath &path);
    QPoint randomItemPosition();
    QColor initialItemColor();
    QColor randomItemColor();
    QToolButton *createToolButton(const QString &toolTip, const QIcon &icon, const char *member);

    void addText(ShapeItem& item);

    QPointF mOrigin;
    QSizeF mItem;
    QSize mDiagramSize;

    QList<ShapeItem> mShapeItems;

    QPainterPath mTitlePath;

    QPainterPath mHeadlinePath;
    QPainterPath mAddressPath;
    QPainterPath mActionPath;
    QPainterPath mDelayPath;

    QPainterPath mAddPath;

    QPainterPath mCirclePath;
    QPainterPath mTrianglePath;

    QPainterPath mHexagonPath;

    QPainterPath mItemConnectorPath;

    QPoint mPreviousPosition;
    ShapeItem * mSelectedItem;

    //QToolButton *newCircleButton;
    //QToolButton *newSquareButton;
    //QToolButton *newTriangleButton;

    QFont mFont;

    ShapeAddDialog * mShapeAddDialog;
    ShapeEditDialog * mShapeEditDialog;
};

#endif
