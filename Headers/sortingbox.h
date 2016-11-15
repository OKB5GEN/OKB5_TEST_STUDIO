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

//private slots:
public slots:
//    void createNewCircle();
//    void createNewSquare();
//    void createNewTriangle();

private slots:
    void addItem(ShapeTypes id, const QPoint& pos);

private:
    void connectItems(const QPoint& pos1, const QPoint& pos2, int addItemCount);
    void drawSilhouette();

    int updateButtonGeometry(QToolButton *button, int x, int y);
    void createShapeItem(const QPainterPath &path, const QString &toolTip, const QPoint &pos, const QColor &color, ShapeTypes type);
    int itemAt(const QPoint &pos);
    void moveItemTo(const QPoint &pos);
    QPoint initialItemPosition(const QPainterPath &path);
    QPoint randomItemPosition();
    QColor initialItemColor();
    QColor randomItemColor();
    QToolButton *createToolButton(const QString &toolTip, const QIcon &icon, const char *member);

    QPointF mOrigin;
    QSizeF mItem;
    QSize mDiagramSize;

    QList<ShapeItem> mShapeItems;

    QPainterPath mTitlePath;

    QPainterPath mHeadlinePath;
    QPainterPath mAddressPath;

    QPainterPath mAddPath;

    QPainterPath mCirclePath;
    QPainterPath mSquarePath;
    QPainterPath mTrianglePath;

    QPainterPath mHexagonPath;

    QPainterPath mItemConnectorPath;

    QPoint mPreviousPosition;
    ShapeItem * mSelectedItem;

    //QToolButton *newCircleButton;
    //QToolButton *newSquareButton;
    //QToolButton *newTriangleButton;

    ShapeAddDialog * mShapeAddDialog;
};

#endif
