#include <QtWidgets>
#include <stdlib.h>
#include "Headers/sortingbox.h"

namespace
{
    static const int CELL_WIDTH = 150;
    static const int CELL_HEIGHT = 60;
}

SortingBox::SortingBox()
{
    setMouseTracking(true);
    setBackgroundRole(QPalette::Base);
    mSelectedItem = 0;

    //newCircleButton = createToolButton(tr("New Circle"), QIcon(":/images/circle.png"), SLOT(createNewCircle()));
    //newSquareButton = createToolButton(tr("New Square"), QIcon(":/images/square.png"), SLOT(createNewSquare()));
    //newTriangleButton = createToolButton(tr("New Triangle"), QIcon(":/images/triangle.png"), SLOT(createNewTriangle()));

    mTitlePath.addRoundedRect(QRectF(0, 0, CELL_WIDTH, CELL_HEIGHT), CELL_HEIGHT / 2, CELL_HEIGHT / 2);

    mAddPath.addEllipse(QRect((CELL_WIDTH - CELL_HEIGHT) / 2, 0, CELL_HEIGHT, CELL_HEIGHT));
    mAddPath.moveTo(CELL_WIDTH / 2, CELL_HEIGHT / 6);
    mAddPath.lineTo(CELL_WIDTH / 2, CELL_HEIGHT * 5 / 6);
    mAddPath.moveTo(CELL_HEIGHT / 6 + (CELL_WIDTH - CELL_HEIGHT) / 2, CELL_HEIGHT / 2);
    mAddPath.lineTo((CELL_WIDTH + CELL_HEIGHT) / 2 - CELL_HEIGHT / 6, CELL_HEIGHT / 2);


    mCirclePath.addEllipse(QRect(0, 0, CELL_HEIGHT, CELL_HEIGHT));
    mSquarePath.addRect(QRect(0, 0, CELL_WIDTH, CELL_HEIGHT));

    qreal x = mTrianglePath.currentPosition().x();
    qreal y = mTrianglePath.currentPosition().y();
    mTrianglePath.moveTo(x + 120 / 2, y);
    mTrianglePath.lineTo(0, 100);
    mTrianglePath.lineTo(120, 100);
    mTrianglePath.lineTo(x + 120 / 2, y);

    qreal x1 = mHexagonPath.currentPosition().x();
    qreal y1 = mHexagonPath.currentPosition().y();

    mHexagonPath.moveTo(x1, y1 + CELL_HEIGHT / 2);
    mHexagonPath.lineTo(CELL_WIDTH / 6, CELL_HEIGHT);
    mHexagonPath.lineTo(CELL_WIDTH * 5 / 6, CELL_HEIGHT);
    mHexagonPath.lineTo(CELL_WIDTH, CELL_HEIGHT / 2);
    mHexagonPath.lineTo(CELL_WIDTH * 5 / 6, 0);
    mHexagonPath.lineTo(CELL_WIDTH / 6, 0);
    mHexagonPath.lineTo(x1, y1 + CELL_HEIGHT / 2);

    qreal x2 = mHeadlinePath.currentPosition().x();
    qreal y2 = mHeadlinePath.currentPosition().y();

    mHeadlinePath.moveTo(x2, y2);
    mHeadlinePath.lineTo(0, CELL_HEIGHT * 2 / 3);
    mHeadlinePath.lineTo(CELL_WIDTH / 2, CELL_HEIGHT);
    mHeadlinePath.lineTo(CELL_WIDTH, CELL_HEIGHT * 2 / 3);
    mHeadlinePath.lineTo(CELL_WIDTH, 0);
    mHeadlinePath.lineTo(x2, y2);

    qreal x3 = mAddressPath.currentPosition().x();
    qreal y3 = mAddressPath.currentPosition().y();

    mAddressPath.moveTo(x3, y3 + CELL_HEIGHT / 3);
    mAddressPath.lineTo(0, CELL_HEIGHT);
    mAddressPath.lineTo(CELL_WIDTH, CELL_HEIGHT);
    mAddressPath.lineTo(CELL_WIDTH, CELL_HEIGHT / 3);
    mAddressPath.lineTo(CELL_WIDTH / 2, 0);
    mAddressPath.lineTo(x3, y3 + CELL_HEIGHT / 3);

    setWindowTitle(tr("Tool Tips"));
    resize(1000, 600);

    QPoint currentPos(CELL_WIDTH, CELL_HEIGHT);

    createShapeItem(mTitlePath, tr("Title"), currentPos /*initialItemPosition(mCirclePath)*/, initialItemColor());
    currentPos += QPoint(0, CELL_HEIGHT);

    createShapeItem(mHeadlinePath, tr("Headline"), currentPos /*initialItemPosition(mCirclePath)*/, initialItemColor());
    currentPos += QPoint(0, CELL_HEIGHT);

    createShapeItem(mAddPath, tr("Add"), currentPos /*initialItemPosition(mCirclePath)*/, initialItemColor());
    currentPos += QPoint(0, CELL_HEIGHT);

    createShapeItem(mAddressPath, tr("Address"), currentPos /*initialItemPosition(mCirclePath)*/, initialItemColor());
    currentPos += QPoint(0, CELL_HEIGHT);

    createShapeItem(mHeadlinePath, tr("Headline END"), QPoint(2 * CELL_WIDTH, 2 * CELL_HEIGHT) /*initialItemPosition(mCirclePath)*/, initialItemColor());
    createShapeItem(mTitlePath, tr("Title END"), QPoint(2 * CELL_WIDTH, 3 * CELL_HEIGHT) /*initialItemPosition(mCirclePath)*/, initialItemColor());

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
    createShapeItem(mCirclePath, tr("Circle <%1>").arg(++count), randomItemPosition(), randomItemColor());
}

void SortingBox::createNewSquare()
{
    static int count = 1;
    createShapeItem(mSquarePath, tr("Square <%1>").arg(++count), randomItemPosition(), randomItemColor());
}

void SortingBox::createNewTriangle()
{
    static int count = 1;
    createShapeItem(mTrianglePath, tr("Triangle <%1>").arg(++count), randomItemPosition(), randomItemColor());
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

void SortingBox::createShapeItem(const QPainterPath &path, const QString &toolTip, const QPoint &pos, const QColor &color)
{
    ShapeItem shapeItem;
    shapeItem.setPath(path);
    shapeItem.setToolTip(toolTip);
    shapeItem.setPosition(pos);
    shapeItem.setColor(color);
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
    return QColor::fromRgba(0x00ffffff);
}

QColor SortingBox::randomItemColor()
{
    return QColor::fromHsv(qrand() % 256, 255, 190);
}

