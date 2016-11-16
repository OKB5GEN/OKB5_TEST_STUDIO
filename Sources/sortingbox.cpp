#include <QtWidgets>
#include <QDebug>
#include <stdlib.h>
#include "Headers/sortingbox.h"
#include "Headers/shapeadddialog.h"
#include "Headers/shapeeditdialog.h"

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
    mShapeAddDialog = new ShapeAddDialog(this);
    mShapeEditDialog = new ShapeEditDialog(this);

    setMouseTracking(true);
    setBackgroundRole(QPalette::Base);
    mSelectedItem = 0;

    mOrigin.setX(mItem.width());
    mOrigin.setY(mItem.height());

    //newCircleButton = createToolButton(tr("New Circle"), QIcon(":/images/circle.png"), SLOT(createNewCircle()));
    //newSquareButton = createToolButton(tr("New Square"), QIcon(":/images/square.png"), SLOT(createNewSquare()));
    //newTriangleButton = createToolButton(tr("New Triangle"), QIcon(":/images/triangle.png"), SLOT(createNewTriangle()));

    mTitlePath.addRoundedRect(QRectF(CELL.width(), CELL.height(), mItem.width() - 2 * CELL.width(), mItem.height() - 2 * CELL.height()), (mItem.height() - 2 * CELL.height()) / 2, (mItem.height() - 2 * CELL.height()) / 2);

    mAddPath.addEllipse(QRect((CELL.width() - CELL.height()) / 2, 0, CELL.height(), CELL.height()));
    mAddPath.moveTo(CELL.width() / 2, CELL.height() / 6);
    mAddPath.lineTo(CELL.width() / 2, CELL.height() * 5 / 6);
    mAddPath.moveTo(CELL.width() / 6 + (CELL.width() - CELL.height()) / 2, CELL.height() / 2);
    mAddPath.lineTo((CELL.width() + CELL.height()) / 2 - CELL.height() / 6, CELL.height() / 2);

    mCirclePath.addEllipse(QRect(CELL.width(), CELL.height(), mItem.width() - 2 * CELL.width(), mItem.height() - 2 * CELL.height()));
    mActionPath.addRect(QRect(CELL.width(), CELL.height(), mItem.width() - 2 * CELL.width(), mItem.height() - 2 * CELL.height()));

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

    addItem(ShapeTypes::TITLE, QPoint(0, 0));
    addItem(ShapeTypes::HEADLINE, QPoint(0, 1));
    addItem(ShapeTypes::ADDRESS, QPoint(0, 2));
    addItem(ShapeTypes::HEADLINE, QPoint(1, 1));
    addItem(ShapeTypes::TITLE, QPoint(1, 2));

    drawSilhouette();
    connectItems(QPoint(0, 0), QPoint(0, 1), 0);
    connectItems(QPoint(0, 1), QPoint(0, 2), 1);
    connectItems(QPoint(1, 1), QPoint(1, 2), 0);
}

SortingBox::~SortingBox()
{

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
            static bool inserted = false; // remove this shit

            if (!inserted && mShapeItems[index].toolTip() == "AddItem") // TODO remove this hack, and implement normally
            {
                mShapeAddDialog->exec();
                if (mShapeAddDialog->result() == QDialog::Accepted)
                {
                    ShapeTypes shapeType = mShapeAddDialog->shapeType();
                    QPoint insertionCell = mShapeItems[index].cell();
                    insertItem(shapeType, insertionCell, index);
                    inserted = true;
                }
            }

            /*
            int TODO; // split shapes on movable selectable
            bool movable = false;
            bool selectable = false;

            if (selectable)
            {
                mSelectedItem = &mShapeItems[index];
                mPreviousPosition = event->pos();
                mShapeItems.move(index, mShapeItems.size() - 1);
                update();
            }
            */
        }
    }
}

void SortingBox::mouseDoubleClickEvent(QMouseEvent *event)
{
    //remember that mousePressEvent will be called first!

    if (event->button() == Qt::LeftButton)
    {
        int index = itemAt(event->pos());
        if (index != -1)
        {
            ShapeTypes type = mShapeItems[index].type();
            if (mShapeItems[index].type() < ShapeTypes::TOTAL_COUNT)
            {
                mShapeEditDialog->exec();
                if (mShapeEditDialog->result() == QDialog::Accepted)
                {
                    // TODO Apply settings in GUI (write text)
                }
            }
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

int SortingBox::itemAt(const QPoint &pos)
{
    for (int i = mShapeItems.size() - 1; i >= 0; --i)
    {
        const ShapeItem &item = mShapeItems[i];

        // TODO hack for excluding "service" shapes
        if (item.type() == ShapeTypes::SILHOUETTE_ARROW
           || item.type() == ShapeTypes::ARROW
           || item.type() == ShapeTypes::CONNECT_LINE)
        {
            continue;
        }


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

void SortingBox::createShapeItem(const QPainterPath &path, const QString &toolTip, const QPoint &pos, const QColor &color, ShapeTypes type, const QPoint& cell)
{
    bool found = false;

    if (toolTip == "Silhouette" || toolTip == "Arrow") // replace if exist TODO remove
    {
        for (int i = 0, sz = mShapeItems.size(); i < sz; ++i)
        {
            if (mShapeItems[i].toolTip() == toolTip)
            {
                found = true;
                mShapeItems[i].setPath(path);
                mShapeItems[i].setToolTip(toolTip);
                mShapeItems[i].setPosition(pos);
                mShapeItems[i].setColor(color);
                mShapeItems[i].setType(type);
                mShapeItems[i].setCell(cell);
                break;
            }
        }
    }

    if (!found)
    {
        ShapeItem shapeItem;
        shapeItem.setPath(path);
        shapeItem.setToolTip(toolTip);
        shapeItem.setPosition(pos);
        shapeItem.setColor(color);
        shapeItem.setType(type);
        shapeItem.setCell(cell);
        mShapeItems.append(shapeItem);
    }

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

void SortingBox::addItem(ShapeTypes id, const QPoint& pos)
{
    QPoint p(mOrigin.x() + pos.x() * mItem.width(), mOrigin.y() + pos.y() * mItem.height());

    qDebug("AddItem to pos (%i, %i) diag sz before is (%i, %i)", pos.x(), pos.y(), mDiagramSize.width(), mDiagramSize.height());
    // TODO неправильная логика при вставке в середину, тут только случай вставки в конец!!!

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
    case ShapeTypes::TITLE:
        createShapeItem(mTitlePath, "Tooltip", p, initialItemColor(), id, pos);
        break;
    case ShapeTypes::HEADLINE:
        createShapeItem(mHeadlinePath, "Tooltip", p, initialItemColor(), id, pos);
        break;
    case ShapeTypes::ADDRESS:
        createShapeItem(mAddressPath, "Tooltip", p, initialItemColor(), id, pos);
        break;
    case ShapeTypes::ACTION:
        createShapeItem(mActionPath, "Tooltip", p, initialItemColor(), id, pos);
        break;

    default:
        {
            QMessageBox::warning(this, tr("Application"), tr("Shape not implemented %1.").arg(int(id)));
            return;
        }
        break;
    }
}

void SortingBox::connectItems(const QPoint& pos1, const QPoint& pos2, int addItemCount)
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

        QPainterPath itemConnectorPath;
        itemConnectorPath.moveTo(x, y - CELL.height());
        itemConnectorPath.lineTo(x, y + CELL.height());

        createShapeItem(itemConnectorPath, "Connector", pos, initialItemColor(), ShapeTypes::CONNECT_LINE, QPoint(0, 0));

        // TODO hardcode
        if (addItemCount == 1)
        {
            QPainterPath addItemPath;

            addItemPath.moveTo(QPoint(x, y));
            qreal radius = qMin(CELL.width(), CELL.height()) / 2;

            addItemPath.addEllipse(QRectF(-radius, -radius, radius * 2, radius * 2));
            createShapeItem(addItemPath, "AddItem", QPoint(x, y), QColor::fromRgba(0xff00ff00), ShapeTypes::VALENCY_POINT, pos2);
        }
    }
}

void SortingBox::drawSilhouette()
{
    QPoint bottomRight(INT_MIN, INT_MIN);
    QPoint bottomLeft(mOrigin.x(), (mDiagramSize.height() + 1) * mItem.height());
    QPoint topLeft(mOrigin.x(), mOrigin.y() + mItem.height());
    QPoint topRight(INT_MIN, INT_MAX);


    qDebug("Silh Bottom left is (%i; %i)", bottomLeft.x(), bottomLeft.y());

    QPainterPath silhouette;

    foreach (ShapeItem shapeItem, mShapeItems)
    {
        if (shapeItem.type() == ShapeTypes::ADDRESS)
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
        else if (shapeItem.type() == ShapeTypes::HEADLINE)
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

    createShapeItem(silhouette, "Silhouette", QPoint(0, 0), QColor::fromRgba(0x00ffffff), ShapeTypes::SILHOUETTE_ARROW, QPoint(0, 0));

    // draw arrow
    QPainterPath arrow;
    QPoint pos;
    pos.setX(topLeft.x() + mItem.width() / 2);
    pos.setY(topLeft.y());
    arrow.moveTo(pos);
    arrow.lineTo(QPoint(pos.x() - CELL.width(), pos.y() + CELL.height() / 4));
    arrow.lineTo(QPoint(pos.x() - CELL.width(), pos.y() - CELL.height() / 4));
    arrow.lineTo(pos);

    createShapeItem(arrow, "Arrow", QPoint(0, 0), QColor::fromRgba(0xff000000), ShapeTypes::ARROW, QPoint(0, 0));
}


void SortingBox::insertItem(ShapeTypes id, const QPoint& pos, int shapeAddItemIndex)
{
    /*
     * Здесь должен быть какой-то хитрожопый алгоритм вставки итема
     * Притом сложные формы типа question или switch будут вставляться более хитрожопо,
     * так как помимо строк будут вставляться еще и столбцы
     *
     * Попробую описать примерный алгоритм вставки простой формы (для демки этого хватит):
     * Вставляется форма ТИП в ячейку (X, Y)
     * Все элементы, расположенные по Y в ОБЩЕМ случае сдвигаются вниз
     *
    */

    for (int i = 0, sz = mShapeItems.size(); i < sz; ++i)
    {
        if (i == shapeAddItemIndex) // skip add item index
        {
            continue;
        }

        ShapeTypes type = mShapeItems[i].type();
        QPoint cell = mShapeItems[i].cell();

        if (id < ShapeTypes::DRAKON_ELEMENTS_COUNT && cell.y() >= pos.y() && cell.x() == pos.x()) // TODO: Х - только для сдвига по столбцу
        {
            QPoint position = mShapeItems[i].position();
            position.setY(position.y() + mItem.height());
            mShapeItems[i].setPosition(position);
            qDebug("Set shape %i pos to (%i; %i)", int(id), position.x(), position.y());
        }
    }

    addItem(id, pos);
    mDiagramSize.setHeight(mDiagramSize.height() + 1);

    connectItems(pos, QPoint(pos.x(), pos.y() + 1), 0);

    drawSilhouette();

    //TODO update connectors
    update();
}
