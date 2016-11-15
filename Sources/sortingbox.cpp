#include <QtWidgets>
#include <stdlib.h>
#include "Headers/sortingbox.h"

/*
 * Здесь мы создаем формы для циклограммы
 *
 * (0;0) - это левый верхний угол. Ось Х идет вправо, ось Y идет вниз
 * Область логически представляет собой сетку из квадратиков.
 * Под каждый ДРАКОН-элемент циклограммы отводится прямоугольник из W на H квадратиков
 * Вокруг собственно элемента дорожка толщиной в 1 квадратик.
 * Дорожка нужна в качестве бокового и вертикального интервала между элементами
 * В дорожку на вертикальном интервале ставится валентная точка (точка, куда можно что-либо добавить)
 * Снаружи класса управление идет по индексам элементов в сетке, а не по квадратикам
*/

namespace
{
    static const QSizeF CELL = QSizeF(30, 30);
    static const int CELLS_PER_ITEM_V = 4;
    static const int CELLS_PER_ITEM_H = 8;
}

SortingBox::SortingBox():
    mItem(CELL.width() * CELLS_PER_ITEM_H, CELL.height() * CELLS_PER_ITEM_V),
    mDiagramSize(0, 0)
{
    setMouseTracking(true);
    setBackgroundRole(QPalette::Base);
    mSelectedItem = 0;

    mOrigin.setX(mItem.width());
    mOrigin.setY(mItem.height());

    //newCircleButton = createToolButton(tr("New Circle"), QIcon(":/images/circle.png"), SLOT(createNewCircle()));
    //newSquareButton = createToolButton(tr("New Square"), QIcon(":/images/square.png"), SLOT(createNewSquare()));
    //newTriangleButton = createToolButton(tr("New Triangle"), QIcon(":/images/triangle.png"), SLOT(createNewTriangle()));

    mTitlePath.addRoundedRect(QRectF(CELL.width(), CELL.height(), mItem.width() - 2 * CELL.width(), mItem.height() - 2 * CELL.height()), (mItem.height() - 2 * CELL.height()) / 2, (mItem.height() - 2 * CELL.height()) / 2);

    /*
    mAddPath.addEllipse(QRect((CELL_WIDTH - CELL_HEIGHT) / 2, 0, CELL_HEIGHT, CELL_HEIGHT));
    mAddPath.moveTo(CELL_WIDTH / 2, CELL_HEIGHT / 6);
    mAddPath.lineTo(CELL_WIDTH / 2, CELL_HEIGHT * 5 / 6);
    mAddPath.moveTo(CELL_HEIGHT / 6 + (CELL_WIDTH - CELL_HEIGHT) / 2, CELL_HEIGHT / 2);
    mAddPath.lineTo((CELL_WIDTH + CELL_HEIGHT) / 2 - CELL_HEIGHT / 6, CELL_HEIGHT / 2);
    */


    mCirclePath.addEllipse(QRect(CELL.width(), CELL.height(), mItem.width() - 2 * CELL.width(), mItem.height() - 2 * CELL.height()));
    mSquarePath.addRect(QRect(CELL.width(), CELL.height(), mItem.width() - 2 * CELL.width(), mItem.height() - 2 * CELL.height()));

    mTrianglePath.moveTo(120 / 2, 0);
    mTrianglePath.lineTo(0, 100);
    mTrianglePath.lineTo(120, 100);
    mTrianglePath.lineTo(120 / 2, 0);

    mHexagonPath.moveTo(CELL.width(), mItem.height() / 2);
    mHexagonPath.lineTo(CELL.width() * 2, mItem.height() - CELL.height());
    mHexagonPath.lineTo(mItem.width() - 2 * CELL.width(), mItem.height() - CELL.height());
    mHexagonPath.lineTo(mItem.width() - CELL.width(), mItem.height() / 2);
    mHexagonPath.lineTo(mItem.width() - 2 * CELL.width(), CELL.height());
    mHexagonPath.lineTo(CELL.width() * 2, CELL.height());
    mHexagonPath.lineTo(CELL.width(), mItem.height() / 2);

    mHeadlinePath.moveTo(CELL.width(), CELL.height());
    mHeadlinePath.lineTo(CELL.width(), mItem.height() - CELL.height() * 3 / 2);
    mHeadlinePath.lineTo(mItem.width() / 2, mItem.height() - CELL.height());
    mHeadlinePath.lineTo(mItem.width() - CELL.width(), mItem.height() - CELL.height() * 3 / 2);
    mHeadlinePath.lineTo(mItem.width() - CELL.width(), CELL.height());
    mHeadlinePath.lineTo(CELL.width(), CELL.height());

    mAddressPath.moveTo(CELL.width(), CELL.height() * 3 / 2);
    mAddressPath.lineTo(CELL.width(), mItem.height() - CELL.height());
    mAddressPath.lineTo(mItem.width() - CELL.width(), mItem.height() - CELL.height());
    mAddressPath.lineTo(mItem.width() - CELL.width(), CELL.height() * 3 / 2);
    mAddressPath.lineTo(mItem.width() / 2, CELL.height());
    mAddressPath.lineTo(CELL.width(), CELL.height() * 3 / 2);

    setWindowTitle(tr("Tool Tips"));
    resize(1000, 600);

    addItem(TITLE, QPoint(0, 0));
    addItem(HEADLINE, QPoint(0, 1));
    addItem(ADDRESS, QPoint(0, 2));
    addItem(HEADLINE, QPoint(1, 1));
    addItem(TITLE, QPoint(1, 2));
    connectItems(QPoint(0, 0), QPoint(0, 1));
    connectItems(QPoint(0, 1), QPoint(0, 2));
    connectItems(QPoint(1, 1), QPoint(1, 2));
    drawSilhouette();

    //createShapeItem(mAddPath, tr("Add"), currentPos /*initialItemPosition(mCirclePath)*/, initialItemColor());
    //currentPos += QPoint(0, item.height());
    //createShapeItem(mAddressPath, tr("Address"), currentPos /*initialItemPosition(mCirclePath)*/, initialItemColor());
    //currentPos += QPoint(0, CELL_HEIGHT);
    //createShapeItem(mCirclePath, tr("Circle"), QPoint(0, 0) /*initialItemPosition(mCirclePath)*/, initialItemColor());
    //createShapeItem(mSquarePath, tr("Square"), QPoint(0, 0) /*initialItemPosition(mSquarePath)*/, initialItemColor());
    //createShapeItem(mTrianglePath, tr("Triangle"),QPoint(0, 0) /*initialItemPosition(mTrianglePath)*/, initialItemColor());
    //createShapeItem(mHexagonPath, tr("Hexagon"), QPoint(0, 0) /*initialItemPosition(mHexagonPath)*/, initialItemColor());
    //createShapeItem(mTitlePath, tr("Title"), QPoint(0, 0) /*initialItemPosition(mHexagonPath)*/, initialItemColor());
}

bool SortingBox::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        int index = itemAt(helpEvent->pos());
        if (index != -1)
        {
            QToolTip::showText(helpEvent->globalPos(), mShapeItems[index].toolTip());
        }
        else
        {
            QToolTip::hideText();
            event->ignore();
        }

        return true;
    }

    return QWidget::event(event);
}

void SortingBox::resizeEvent(QResizeEvent * /* event */)
{
    //int margin = style()->pixelMetric(QStyle::PM_DefaultTopLevelMargin);
    //int x = width() - margin;
    //int y = height() - margin;

    //y = updateButtonGeometry(newCircleButton, x, y);
    //y = updateButtonGeometry(newSquareButton, x, y);
    //updateButtonGeometry(newTriangleButton, x, y);
}

void SortingBox::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    foreach (ShapeItem shapeItem, mShapeItems)
    {
        painter.translate(shapeItem.position());
        painter.setBrush(shapeItem.color());
        painter.drawPath(shapeItem.path());
        painter.translate(-shapeItem.position());
    }
}

void SortingBox::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        int index = itemAt(event->pos());
        if (index != -1)
        {
            mSelectedItem = &mShapeItems[index];
            mPreviousPosition = event->pos();
            mShapeItems.move(index, mShapeItems.size() - 1);
            update();
        }
    }
}

void SortingBox::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && mSelectedItem)
    {
        moveItemTo(event->pos());
    }
}

void SortingBox::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && mSelectedItem)
    {
        moveItemTo(event->pos());
        mSelectedItem = 0;
    }
}

void SortingBox::createNewCircle()
{
    static int count = 1;
    createShapeItem(mCirclePath, tr("Circle <%1>").arg(++count), randomItemPosition(), randomItemColor(), -1);
}

void SortingBox::createNewSquare()
{
    static int count = 1;
    createShapeItem(mSquarePath, tr("Square <%1>").arg(++count), randomItemPosition(), randomItemColor(), -1);
}

void SortingBox::createNewTriangle()
{
    static int count = 1;
    createShapeItem(mTrianglePath, tr("Triangle <%1>").arg(++count), randomItemPosition(), randomItemColor(), -1);
}

int SortingBox::itemAt(const QPoint &pos)
{
    for (int i = mShapeItems.size() - 1; i >= 0; --i)
    {
        const ShapeItem &item = mShapeItems[i];
        if (item.path().contains(pos - item.position()))
        {
            return i;
        }
    }
    return -1;
}

void SortingBox::moveItemTo(const QPoint &pos)
{
    QPoint offset = pos - mPreviousPosition;
    mSelectedItem->setPosition(mSelectedItem->position() + offset);
    mPreviousPosition = pos;
    update();
}

int SortingBox::updateButtonGeometry(QToolButton *button, int x, int y)
{
    QSize size = button->sizeHint();
    button->setGeometry(x - size.rwidth(), y - size.rheight(), size.rwidth(), size.rheight());

    return y - size.rheight() - style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
}

void SortingBox::createShapeItem(const QPainterPath &path, const QString &toolTip, const QPoint &pos, const QColor &color, int type)
{
    ShapeItem shapeItem;
    shapeItem.setPath(path);
    shapeItem.setToolTip(toolTip);
    shapeItem.setPosition(pos);
    shapeItem.setColor(color);
    shapeItem.setType(type);
    mShapeItems.append(shapeItem);
    update();
}

QToolButton *SortingBox::createToolButton(const QString &toolTip, const QIcon &icon, const char *member)
{
    QToolButton *button = new QToolButton(this);
    button->setToolTip(toolTip);
    button->setIcon(icon);
    button->setIconSize(QSize(32, 32));
    connect(button, SIGNAL(clicked()), this, member);

    return button;
}

QPoint SortingBox::initialItemPosition(const QPainterPath &path)
{
    int x;
    int y = (height() - (int)path.controlPointRect().height()) / 2;
    if (mShapeItems.empty())
    {
        x = ((3 * width()) / 2 - (int)path.controlPointRect().width()) / 2;
    }
    else
    {
        x = (width() / mShapeItems.size() - (int)path.controlPointRect().width()) / 2;
    }

    return QPoint(x, y);
}

QPoint SortingBox::randomItemPosition()
{
    return QPoint(qrand() % (width() - 120), qrand() % (height() - 120));
}

QColor SortingBox::initialItemColor()
{
    //return QColor::fromHsv(((shapeItems.size() + 1) * 85) % 256, 255, 190);
    return QColor::fromRgba(0xffffffff);
}

QColor SortingBox::randomItemColor()
{
    return QColor::fromHsv(qrand() % 256, 255, 190);
}

void SortingBox::addItem(SortingBox::IconID id, const QPoint& pos)
{
    QPoint p(mOrigin.x() + pos.x() * mItem.width(), mOrigin.y() + pos.y() * mItem.height());

    if (pos.x() + 1 > mDiagramSize.width())
    {
        mDiagramSize.setWidth(pos.x() + 1);
    }

    if (pos.y() + 1 > mDiagramSize.height())
    {
        mDiagramSize.setHeight(pos.y() + 1);
    }

    switch (id)
    {
    case TITLE:
        createShapeItem(mTitlePath, "Tooltip", p, initialItemColor(), id);
        break;
    case HEADLINE:
        createShapeItem(mHeadlinePath, "Tooltip", p, initialItemColor(), id);
        break;

    case ADDRESS:
        createShapeItem(mAddressPath, "Tooltip", p, initialItemColor(), id);
        break;

    default:
        {
            QMessageBox::warning(this, tr("Application"), tr("Shape not implemented %1.").arg(id));
            return;
        }
        break;
    }
}

void SortingBox::connectItems(const QPoint& pos1, const QPoint& pos2)
{
    // TODO пока полухардкод для соседних элементов по вертикали
    if (pos1.x() == pos2.x() && qAbs(pos2.y() - pos1.y()) == 1)
    {
        QPoint pos = pos1;
        if (pos2.y() < pos1.y())
        {
            pos = pos2;
        }

        qreal x = mOrigin.x() + pos.x() * mItem.width() + mItem.width() / 2;
        qreal y = mOrigin.y() + pos.y() * mItem.height() + mItem.height();

        mItemConnectorPath.moveTo(x, y - CELL.height());
        mItemConnectorPath.lineTo(x, y + CELL.height());

        createShapeItem(mItemConnectorPath, "Connector", pos, initialItemColor(), -1);
    }
}

void SortingBox::drawSilhouette()
{
    QPoint bottomRight(INT_MIN, INT_MIN);
    QPoint bottomLeft(mOrigin.x(), (mDiagramSize.height() + 1) * mItem.height());
    QPoint topLeft(mOrigin.x(), mOrigin.y() + mItem.height());
    QPoint topRight(INT_MIN, INT_MAX);

    QPainterPath silhouette;

    foreach (ShapeItem shapeItem, mShapeItems)
    {
        if (shapeItem.type() == ADDRESS)
        {
            qreal x = shapeItem.position().x() + mItem.width() / 2;
            qreal y = shapeItem.position().y() + mItem.height() - CELL.height();
            QPoint src(x, y);
            QPoint dest(x, y + CELL.height());
            silhouette.moveTo(src);
            silhouette.lineTo(dest);
            if (dest.x() > bottomRight.x())
            {
                bottomRight = dest;
            }
        }
        else if (shapeItem.type() == HEADLINE)
        {
            qreal x = shapeItem.position().x() + mItem.width() / 2;
            qreal y = shapeItem.position().y() + CELL.height();
            QPoint src(x, y);
            QPoint dest(x, y - CELL.height());
            silhouette.moveTo(src);
            silhouette.lineTo(dest);
            if (dest.x() > topRight.x())
            {
                topRight = dest;
            }
        }
    }

    silhouette.moveTo(bottomRight);
    silhouette.lineTo(bottomLeft);
    silhouette.lineTo(topLeft);
    silhouette.lineTo(topRight);

    createShapeItem(silhouette, "Connector", QPoint(0, 0), QColor::fromRgba(0x00ffffff), -1);

    // draw arrow
    QPainterPath arrow;
    QPoint pos;
    pos.setX(topLeft.x() + mItem.width() / 2);
    pos.setY(topLeft.y());
    arrow.moveTo(pos);
    arrow.lineTo(QPoint(pos.x() - CELL.width(), pos.y() + CELL.height() / 4));
    arrow.lineTo(QPoint(pos.x() - CELL.width(), pos.y() - CELL.height() / 4));
    arrow.lineTo(pos);

    createShapeItem(arrow, "Arrow", QPoint(0, 0), QColor::fromRgba(0xff000000), -1);
}
