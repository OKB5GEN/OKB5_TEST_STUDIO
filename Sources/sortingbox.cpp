#include <QtWidgets>
#include <QDebug>
#include <stdlib.h>
#include "Headers/sortingbox.h"
#include "Headers/shapeadddialog.h"
#include "Headers/shapeeditdialog.h"
#include "Headers/cyclogram.h"
#include "Headers/cell.h"
#include "Headers/command.h"
#include "Headers/commands/cmd_title.h"

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

    QString defaultText = "Default";
    mFont.setPointSize(16);
    mFont.setFamily("Arial");

    setMouseTracking(true);
    setBackgroundRole(QPalette::Base);
    mSelectedItem = 0;

    // Set cyclogram origin
    mOrigin.setX(mItem.width() / 4);
    mOrigin.setY(0);

    setWindowTitle(tr("Tool Tips"));
    resize(1000, 600);

    /*
    connectItems(QPoint(0, 0), QPoint(0, 1), 0);
    connectItems(QPoint(0, 1), QPoint(0, 2), 1);
    connectItems(QPoint(1, 1), QPoint(1, 2), 0);
    */
}

SortingBox::~SortingBox()
{

}

bool SortingBox::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        int index = itemAt(helpEvent->pos(), mCommands);
        if (index != -1)
        {
            QToolTip::showText(helpEvent->globalPos(), mCommands[index].toolTip());
        }
        else
        {
            int index = itemAt(helpEvent->pos(), mValencyPoints);
            if (index != -1)
            {
                QToolTip::showText(helpEvent->globalPos(), mValencyPoints[index].toolTip());
            }
            else
            {
                QToolTip::hideText();
                event->ignore();
            }
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

    drawItems(mCommands, painter);
    drawItems(mSihlouette, painter);
    drawItems(mConnectors, painter);
    drawItems(mValencyPoints, painter);
}

void SortingBox::drawItems(QList<ShapeItem>& items, QPainter& painter)
{
    foreach (ShapeItem shapeItem, items)
    {
        painter.translate(shapeItem.position());
        painter.setBrush(shapeItem.color());
        painter.drawPath(shapeItem.path());
        painter.drawPath(shapeItem.textPath());
        painter.translate(-shapeItem.position());
    }
}

void SortingBox::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        int index = itemAt(event->pos(), mValencyPoints);
        if (index != -1)
        {
            int TODO; // set valency point to dialog to know what can be inserted in this valency point
            mShapeAddDialog->exec();
            if (mShapeAddDialog->result() == QDialog::Accepted)
            {
                ShapeTypes shapeType = mShapeAddDialog->shapeType();
                int TODO2; // insert new command here
                //QPoint insertionCell = mShapeItems[index].cell();
                //insertItem(shapeType, insertionCell, "Delay", index);
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
        int index = itemAt(event->pos(), mCommands);
        if (index == -1)
        {
            return;
        }

        mShapeEditDialog->setCommand(mCommands[index].command());
        mShapeEditDialog->exec();

        if (mShapeEditDialog->result() == QDialog::Accepted)
        {
            addText(mCommands[index]);
        }
    }
}

void SortingBox::mouseMoveEvent(QMouseEvent *event)
{
    if (mSelectedItem && (event->buttons() & Qt::LeftButton))
    {
        moveItemTo(event->pos());
    }
}

void SortingBox::mouseReleaseEvent(QMouseEvent *event)
{
    if (mSelectedItem && event->button() == Qt::LeftButton)
    {
        moveItemTo(event->pos());
        mSelectedItem = 0;
    }
}

int SortingBox::itemAt(const QPoint &pos, const QList<ShapeItem>& items)
{
    for (int i = items.size() - 1; i >= 0; --i)
    {
        const ShapeItem &item = items[i];
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
/*
int SortingBox::updateButtonGeometry(QToolButton *button, int x, int y)
{
    QSize size = button->sizeHint();
    button->setGeometry(x - size.rwidth(), y - size.rheight(), size.rwidth(), size.rheight());

    return y - size.rheight() - style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
}
*/

void SortingBox::createCommandShape(Command* cmd, const QPoint& cell)
{
    QPoint pos(mOrigin.x() + cell.x() * mItem.width(), mOrigin.y() + cell.y() * mItem.height());

    // TODO неправильная логика при вставке в середину, тут только случай вставки в конец!!!
    if (cell.x() + 1 > mDiagramSize.width())
    {
        mDiagramSize.setWidth(cell.x() + 1);
    }

    if (cell.y() + 1 > mDiagramSize.height())
    {
        mDiagramSize.setHeight(cell.y() + 1);
    }


    ShapeItem shapeItem;
    shapeItem.setCommand(cmd);
    shapeItem.setPath(createPath(cmd));
    shapeItem.setToolTip(tr("Tooltip"));
    shapeItem.setPosition(pos);
    shapeItem.setColor(QColor::fromRgba(0xffffffff));
    shapeItem.setCell(cell);
    addText(shapeItem);
    mCommands.append(shapeItem);

    update();
}

void SortingBox::createConnectorShape(const QPainterPath &path, const QPoint &pos)
{
    ShapeItem shapeItem;
    shapeItem.setPath(path);
    shapeItem.setPosition(pos);
    shapeItem.setColor(QColor::fromRgba(0xffffffff));
    mConnectors.append(shapeItem);
    update();
}

void SortingBox::createValencyPointShape(const QPoint &pos, const QPoint& cell)
{
    QPainterPath path;
    path.moveTo(pos);
    qreal radius = qMin(CELL.width(), CELL.height()) / 2;
    path.addEllipse(QRectF(-radius, -radius, radius * 2, radius * 2));

    ShapeItem shapeItem;
    shapeItem.setPath(path);
    shapeItem.setToolTip(tr("Valency point"));
    shapeItem.setPosition(pos);
    shapeItem.setColor(QColor::fromRgba(0xff00ff00));
    shapeItem.setCell(cell);
    mValencyPoints.append(shapeItem);

    update();
}

void SortingBox::addText(ShapeItem& item)
{
    QPainterPath textPath;
    QFontMetrics fm(mFont);
    QRect textRect = fm.boundingRect(item.command()->text());
    qreal x = (mItem.width() - textRect.width()) / 2;
    qreal y = (mItem.height() + textRect.height()) / 2;
    textPath.addText(x, y, mFont, item.command()->text());
    item.setTextPath(textPath);
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

        createConnectorShape(itemConnectorPath, pos);

        // TODO hardcode
        if (addItemCount == 1)
        {
            createValencyPointShape(QPoint(x, y), pos2);
        }
    }
}

void SortingBox::drawSilhouette()
{
    mSihlouette.clear();

    QPoint bottomRight(INT_MIN, INT_MIN);
    QPoint bottomLeft(mOrigin.x(), mDiagramSize.height() * mItem.height());
    QPoint topLeft(mOrigin.x(), mOrigin.y() + mItem.height());
    QPoint topRight(INT_MIN, INT_MAX);

    QPainterPath silhouette;

    foreach (ShapeItem shapeItem, mCommands)
    {
        if (shapeItem.command()->type() == ShapeTypes::GO_TO_BRANCH)
        {
            qreal x = shapeItem.position().x() + mItem.width() / 2;
            if (x > bottomRight.x())
            {
                qreal y = shapeItem.position().y() + mItem.height();
                bottomRight.setX(x);
                bottomRight.setY(y);
            }
        }
        else if (shapeItem.command()->type() == ShapeTypes::BRANCH_BEGIN)
        {
            qreal x = shapeItem.position().x() + mItem.width() / 2;
            if (x > topRight.x())
            {
                qreal y = shapeItem.position().y();
                topRight.setX(x);
                topRight.setY(y);
            }
        }
    }

    silhouette.moveTo(bottomRight);
    silhouette.lineTo(bottomLeft);
    silhouette.lineTo(topLeft);
    silhouette.lineTo(topRight);

    ShapeItem sihlouetteItem;
    sihlouetteItem.setPath(silhouette);
    sihlouetteItem.setPosition(QPoint(0, 0));
    sihlouetteItem.setColor(QColor::fromRgba(0x00ffffff));

    // draw arrow
    QPainterPath arrow;
    QPoint pos;
    pos.setX(topLeft.x() + mItem.width() / 2);
    pos.setY(topLeft.y());
    arrow.moveTo(pos);
    arrow.lineTo(QPoint(pos.x() - CELL.width(), pos.y() + CELL.height() / 4));
    arrow.lineTo(QPoint(pos.x() - CELL.width(), pos.y() - CELL.height() / 4));
    arrow.lineTo(pos);

    ShapeItem arrowItem;
    arrowItem.setPath(arrow);
    arrowItem.setPosition(QPoint(0, 0));
    arrowItem.setColor(QColor::fromRgba(0xff000000));

    mSihlouette.push_back(sihlouetteItem);
    mSihlouette.push_back(arrowItem);

    int TODO; // add connector for branch adding
}

void SortingBox::insertItem(ShapeTypes id, const QPoint& pos, const QString& text, int shapeAddItemIndex)
{
    /*
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

    addItem(id, pos, text);
    mDiagramSize.setHeight(mDiagramSize.height() + 1);

    connectItems(pos, QPoint(pos.x(), pos.y() + 1), 0);

    drawSilhouette();

    //TODO update connectors
    update();
    */
}

void SortingBox::load(Cyclogram* cyclogram)
{
    if (!cyclogram)
    {
        return;
    }

    Command* first = cyclogram->first();

    if (!first || first->type() != ShapeTypes::TERMINATOR)
    {
        return;
    }

    QPoint parentCell(0, 0);
    createCommandShape(first, parentCell);
    addChildCommands(first, parentCell);

    drawSilhouette();
}

void SortingBox::addChildCommands(Command* parentCmd, const QPoint& parentCell)
{
    QList<Command*> nextCommands = parentCmd->nextCommands();
    if (nextCommands.empty())
    {
        return;
    }

    if (nextCommands.size() == 1)
    {
        Command* cmd = nextCommands[0];
        QPoint cell(parentCell.x(), parentCell.y() + 1);

        if (cmd->type() == ShapeTypes::BRANCH_BEGIN && parentCmd->type() == ShapeTypes::GO_TO_BRANCH) // not first branch
        {
            cell.setX(mDiagramSize.width());
            cell.setY(1);
        }

        createCommandShape(cmd, cell);

        if (cmd->type() == ShapeTypes::GO_TO_BRANCH)
        {
            if (!isHeadlineExist(cmd))
            {
                addChildCommands(cmd, cell);
            }
        }
        else
        {
            addChildCommands(cmd, cell);
        }

    }
    else // QUESTION or SWITCH-CASE command
    {
        int TODO;
    }
}

bool SortingBox::isHeadlineExist(Command* parentCmd)
{
    int TODO; // is headline exist
    return false;
}

QPainterPath SortingBox::createPath(Command* cmd)
{
    ShapeTypes type = cmd->type();
    QPainterPath path;

    /*
    mAddPath.addEllipse(QRect((CELL.width() - CELL.height()) / 2, 0, CELL.height(), CELL.height()));
    mAddPath.moveTo(CELL.width() / 2, CELL.height() / 6);
    mAddPath.lineTo(CELL.width() / 2, CELL.height() * 5 / 6);
    mAddPath.moveTo(CELL.width() / 6 + (CELL.width() - CELL.height()) / 2, CELL.height() / 2);
    mAddPath.lineTo((CELL.width() + CELL.height()) / 2 - CELL.height() / 6, CELL.height() / 2);

    mCirclePath.addEllipse(QRect(CELL.width(), CELL.height(), mItem.width() - 2 * CELL.width(), mItem.height() - 2 * CELL.height()));
    */

    switch (type)
    {
    case ShapeTypes::TERMINATOR:
        {
            QRectF rect(CELL.width(), CELL.height(), mItem.width() - 2 * CELL.width(), mItem.height() - 2 * CELL.height());
            qreal xRadius = (mItem.height() - 2 * CELL.height()) / 2;
            qreal yRadius = (mItem.height() - 2 * CELL.height()) / 2;
            path.addRoundedRect(rect, xRadius, yRadius);

            // connector
            CmdTitle* titleCmd = qobject_cast<CmdTitle*>(cmd);
            if (titleCmd)
            {
                if (titleCmd->titleType() == CmdTitle::BEGIN)
                {
                    path.moveTo(mItem.width() / 2, mItem.height() - CELL.height());
                    path.lineTo(mItem.width() / 2, mItem.height());
                }
                else
                {
                    path.moveTo(mItem.width() / 2, CELL.height());
                    path.lineTo(mItem.width() / 2, 0);
                }
            }
        }
        break;
    case ShapeTypes::BRANCH_BEGIN:
        {
            path.moveTo(CELL.width(), CELL.height());
            path.lineTo(CELL.width(), mItem.height() - CELL.height() * 3 / 2);
            path.lineTo(mItem.width() / 2, mItem.height() - CELL.height());
            path.lineTo(mItem.width() - CELL.width(), mItem.height() - CELL.height() * 3 / 2);
            path.lineTo(mItem.width() - CELL.width(), CELL.height());
            path.lineTo(CELL.width(), CELL.height());
        }
        break;
    case ShapeTypes::GO_TO_BRANCH:
        {
            path.moveTo(CELL.width(), CELL.height() * 3 / 2);
            path.lineTo(CELL.width(), mItem.height() - CELL.height());
            path.lineTo(mItem.width() - CELL.width(), mItem.height() - CELL.height());
            path.lineTo(mItem.width() - CELL.width(), CELL.height() * 3 / 2);
            path.lineTo(mItem.width() / 2, CELL.height());
            path.lineTo(CELL.width(), CELL.height() * 3 / 2);
        }
        break;
    case ShapeTypes::ACTION:
        {
            path.addRect(QRect(CELL.width(), CELL.height(), mItem.width() - 2 * CELL.width(), mItem.height() - 2 * CELL.height()));
        }
        break;
    case ShapeTypes::DELAY:
        {
            path.moveTo(CELL.width(), CELL.height());
            path.lineTo(CELL.width() * 2, mItem.height() - CELL.height());
            path.lineTo(mItem.width() - 2 * CELL.width(), mItem.height() - CELL.height());
            path.lineTo(mItem.width() - CELL.width(), CELL.height());
            path.lineTo(CELL.width(), CELL.height());
        }
        break;

    case ShapeTypes::QUESTION:
        {
            path.moveTo(CELL.width(), mItem.height() / 2);
            path.lineTo(CELL.width() * 2, mItem.height() - CELL.height());
            path.lineTo(mItem.width() - 2 * CELL.width(), mItem.height() - CELL.height());
            path.lineTo(mItem.width() - CELL.width(), mItem.height() / 2);
            path.lineTo(mItem.width() - 2 * CELL.width(), CELL.height());
            path.lineTo(CELL.width() * 2, CELL.height());
            path.lineTo(CELL.width(), mItem.height() / 2);
        }
        break;
    default:
        break;
    }

    if (cmd->type() != ShapeTypes::TERMINATOR)
    {
        // lower connector
        path.moveTo(mItem.width() / 2, mItem.height() - CELL.height());
        path.lineTo(mItem.width() / 2, mItem.height());
        // upper connector
        path.moveTo(mItem.width() / 2, CELL.height());
        path.lineTo(mItem.width() / 2, 0);
    }

    return path;
}
