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

#include "Headers/cmd_delay_edit_dialog.h"
#include "Headers/commands/cmd_delay.h"

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

    mEditDialogs[ShapeTypes::DELAY] = new CmdDelayEditDialog(this);

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
}

SortingBox::~SortingBox()
{
    clear();
}

void SortingBox::clear()
{
    qDeleteAll(mCommands);
    qDeleteAll(mSihlouette);

    mCurrentCyclogram = Q_NULLPTR;
}

bool SortingBox::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        int index = commandAt(helpEvent->pos());
        if (index != -1)
        {
            QToolTip::showText(helpEvent->globalPos(), mCommands[index]->toolTip());
        }
        else
        {
            ValencyPoint point;
            if (hasValencyPointAt(helpEvent->pos(), point))
            {
                QToolTip::showText(helpEvent->globalPos(), tr("Click to add command"));
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

    drawItems(mSihlouette, painter);
    drawItems(mCommands, painter);
}

void SortingBox::drawItems(QList<ShapeItem*>& items, QPainter& painter)
{
    // draw commands shapes first
    foreach (ShapeItem* shapeItem, items)
    {
        painter.translate(shapeItem->position());
        painter.setBrush(shapeItem->color());
        painter.drawPath(shapeItem->path());
        painter.drawPath(shapeItem->textPath());
        painter.translate(-shapeItem->position());
    }

    // draw valency point secont (to be above the commands shapes)
    foreach (ShapeItem* shapeItem, items)
    {
        painter.translate(shapeItem->position());

        foreach (ValencyPoint point, shapeItem->valencyPoints())
        {
            painter.setBrush(point.color());
            painter.drawPath(point.path());
        }

        painter.translate(-shapeItem->position());
    }
}

void SortingBox::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        int TODO; // if click is inside command create command copy and drag it with cursor

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

        ValencyPoint point;
        if (hasValencyPointAt(event->pos(), point))
        {
            mShapeAddDialog->exec();
            if (mShapeAddDialog->result() == QDialog::Accepted)
            {
                ShapeTypes shapeType = mShapeAddDialog->shapeType();
                addCommand(shapeType, point);
            }
        }
    }
}

void SortingBox::mouseDoubleClickEvent(QMouseEvent *event)
{
    //remember that mousePressEvent will be called first!

    if (event->button() == Qt::LeftButton)
    {
        int index = commandAt(event->pos());
        if (index == -1)
        {
            return;
        }

        showEditDialog(mCommands[index]);
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

int SortingBox::commandAt(const QPoint &pos)
{
    for (int i = mCommands.size() - 1; i >= 0; --i)
    {
        ShapeItem* item = mCommands[i];
        if (item->path().contains(pos - item->position()))
        {
            return i;
        }
    }

    return -1;
}

bool SortingBox::hasValencyPointAt(const QPoint &pos, ValencyPoint& point)
{
    for (int i = mCommands.size() - 1; i >= 0; --i)
    {
        const QList<ValencyPoint>& points = mCommands[i]->valencyPoints();
        for (int j = 0, sz = points.size(); j < sz; ++j)
        {
            if (points[j].path().contains(pos - mCommands[i]->position()))
            {
                point = points[j];
                return true;
            }
        }
    }

    return false;
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

ShapeItem* SortingBox::createCommandShape(Command* cmd, const QPoint& cell)
{
    QPoint pos(mOrigin.x() + cell.x() * mItem.width(), mOrigin.y() + cell.y() * mItem.height());

    int TODO; //update diagram size if adding is to the end of the diagram, REMOVE FROM THIS METHOD
    if (cell.x() + 1 > mDiagramSize.width())
    {
        mDiagramSize.setWidth(cell.x() + 1);
    }

    if (cell.y() + 1 > mDiagramSize.height())
    {
        mDiagramSize.setHeight(cell.y() + 1);
    }

    ShapeItem* shapeItem = new ShapeItem();
    shapeItem->setCommand(cmd);
    shapeItem->setToolTip(tr("Tooltip"));
    shapeItem->setPosition(pos);
    shapeItem->setColor(QColor::fromRgba(0xffffffff));
    shapeItem->setCell(cell);
    shapeItem->setRect(QRect(cell.x(), cell.y(), 1, 1)); // by initial shape rect matches the occupied cell
    shapeItem->setPath(createPath(shapeItem));
    shapeItem->setValencyPoints(createValencyPoints(cmd));
    addText(shapeItem);
    mCommands.append(shapeItem);

    return shapeItem;
}

void SortingBox::addText(ShapeItem* item)
{
    int TODO; // move it to the shape item

    QPainterPath textPath;
    QFontMetrics fm(mFont);
    QRect textRect = fm.boundingRect(item->command()->text());
    qreal x = (mItem.width() - textRect.width()) / 2;
    qreal y = (mItem.height() + textRect.height()) / 2;
    textPath.addText(x, y, mFont, item->command()->text());
    item->setTextPath(textPath);
}

void SortingBox::drawSilhouette()
{
    int TODO; // not good handling
    qDeleteAll(mSihlouette);
    mSihlouette.clear();

    QPoint bottomRight(INT_MIN, INT_MIN);
    QPoint bottomLeft(mOrigin.x(), mDiagramSize.height() * mItem.height());
    QPoint topLeft(mOrigin.x(), mOrigin.y() + mItem.height());
    QPoint topRight(INT_MIN, INT_MAX);

    QPainterPath silhouette;

    foreach (ShapeItem* shapeItem, mCommands)
    {
        if (shapeItem->command()->type() == ShapeTypes::GO_TO_BRANCH)
        {
            qreal x = shapeItem->position().x() + mItem.width() / 2;
            if (x > bottomRight.x())
            {
                qreal y = shapeItem->position().y() + mItem.height();
                bottomRight.setX(x);
                bottomRight.setY(y);
            }
        }
        else if (shapeItem->command()->type() == ShapeTypes::BRANCH_BEGIN)
        {
            qreal x = shapeItem->position().x() + mItem.width() / 2;
            if (x > topRight.x())
            {
                qreal y = shapeItem->position().y();
                topRight.setX(x);
                topRight.setY(y);
            }
        }
    }

    silhouette.moveTo(bottomRight);
    silhouette.lineTo(bottomLeft);
    silhouette.lineTo(topLeft);
    silhouette.lineTo(topRight);

    ShapeItem* sihlouetteItem = new ShapeItem();
    sihlouetteItem->setPath(silhouette);
    sihlouetteItem->setPosition(QPoint(0, 0));
    sihlouetteItem->setColor(QColor::fromRgba(0x00ffffff));

    // draw arrow
    QPainterPath arrow;
    QPoint pos;
    pos.setX(topLeft.x() + mItem.width() / 2);
    pos.setY(topLeft.y());
    arrow.moveTo(pos);
    arrow.lineTo(QPoint(pos.x() - CELL.width(), pos.y() + CELL.height() / 4));
    arrow.lineTo(QPoint(pos.x() - CELL.width(), pos.y() - CELL.height() / 4));
    arrow.lineTo(pos);

    ShapeItem* arrowItem = new ShapeItem();
    arrowItem->setPath(arrow);
    arrowItem->setPosition(QPoint(0, 0));
    arrowItem->setColor(QColor::fromRgba(0xff000000));

    mSihlouette.push_back(sihlouetteItem);
    mSihlouette.push_back(arrowItem);
}

void SortingBox::load(Cyclogram* cyclogram)
{
    clear();

    if (!cyclogram)
    {
        return;
    }

    Command* first = cyclogram->first();

    if (!first || first->type() != ShapeTypes::TERMINATOR)
    {
        return;
    }

    mCurrentCyclogram = cyclogram;
    QPoint parentCell(0, 0);
    createCommandShape(first, parentCell);
    addChildCommands(first, parentCell);

    drawSilhouette();
    update();
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
            if (!isBranchExist(cmd))
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

bool SortingBox::isBranchExist(Command* goToBranchCmd)
{
    foreach (ShapeItem* shape, mCommands)
    {
        Command* command = shape->command();
        if (command->type() == ShapeTypes::BRANCH_BEGIN && command->text() == goToBranchCmd->text())
        {
            return true;
        }
    }

    return false;
}

QPainterPath SortingBox::createPath(ShapeItem* item) const
{
    ShapeTypes type = item->command()->type();
    QPainterPath path;

    QRect itemRect = item->rect();
    QPoint cell = item->cell();

    switch (type)
    {
    case ShapeTypes::TERMINATOR:
        {
            qreal yOffset = (cell.y() - itemRect.top()) * mItem.height();
            qreal radius = (mItem.height() - 2 * CELL.height()) / 2;
            QRectF rect(CELL.width(), CELL.height(), mItem.width() - 2 * CELL.width(), 2 * radius);
            path.addRoundedRect(rect, radius, radius);

            // connector
            CmdTitle* titleCmd = qobject_cast<CmdTitle*>(item->command());
            if (titleCmd)
            {
                if (titleCmd->titleType() == CmdTitle::BEGIN)
                {
                    path.moveTo(mItem.width() / 2, mItem.height() - CELL.height());
                    path.lineTo(mItem.width() / 2, mItem.height());
                }
                else // END terminator
                {
                    path.moveTo(mItem.width() / 2, CELL.height());
                    path.lineTo(mItem.width() / 2, -yOffset);
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
            int TODO; // very complex logics for QUESTION connections drawing will be here

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

    if (type != ShapeTypes::TERMINATOR)
    {
        qreal yOffset = (cell.y() - itemRect.top()) * mItem.height();

        // lower connector
        path.moveTo(mItem.width() / 2, mItem.height() - CELL.height());
        path.lineTo(mItem.width() / 2, mItem.height());
        // upper connector
        path.moveTo(mItem.width() / 2, CELL.height());
        path.lineTo(mItem.width() / 2, -yOffset);
    }

    return path;
}

QList<ValencyPoint> SortingBox::createValencyPoints(Command* cmd)
{
    //
    // Valency points general rules
    //
    // 1. All valency points belongs to any shape
    // 2. All shapes (with except QUESION and BRANCH_BEGIN) can have only one valency point BELOW shape
    // 3. TERMINATOR and GO_TO_BRANCH shapes does not have any valency points
    // 4. BRANCH_BEGIN shape has two valency points: below shape and in top-right corner
    // 5. BRANCH_BEGIN shapes' valency point in top-right corner is for adding new branches only
    // 6. BRANCH_BEGIN shape, that contains "END" TERMINATOR doesn't have top-right valency point
    // 7. QUESTION shape CYCLE contains 3 valency points: below, above and at top-right corner
    // 7. QUESTION shape IF contains 3 valency points: bottom below arrow, bottom above arrow and at bottom-right corner

    // QUESTION shape valency points transformations while adding forms
    //
    // 1.

    QList<ValencyPoint> points;

    ShapeTypes type = cmd->type();

    switch (type)
    {
    case ShapeTypes::BRANCH_BEGIN:
    case ShapeTypes::ACTION:
    case ShapeTypes::DELAY:
        {
            ValencyPoint point = createPoint(QPointF(mItem.width() / 2, mItem.height()));
            points.push_back(point);

            if (type == ShapeTypes::BRANCH_BEGIN && !isCyclogramEndBranch(cmd))
            {
                ValencyPoint point = createPoint(QPointF(mItem.width(), 0));
                points.push_back(point);
            }
        }
        break;

    case ShapeTypes::QUESTION:
        {
            int TODO; // very complex logics will be here
        /*
            path.moveTo(CELL.width(), mItem.height() / 2);
            path.lineTo(CELL.width() * 2, mItem.height() - CELL.height());
            path.lineTo(mItem.width() - 2 * CELL.width(), mItem.height() - CELL.height());
            path.lineTo(mItem.width() - CELL.width(), mItem.height() / 2);
            path.lineTo(mItem.width() - 2 * CELL.width(), CELL.height());
            path.lineTo(CELL.width() * 2, CELL.height());
            path.lineTo(CELL.width(), mItem.height() / 2);
            */
        }
        break;
    default:
        break;
    }

    return points;
}

ValencyPoint SortingBox::createPoint(const QPointF& point)
{
    QPainterPath path;

    qreal crossSize = 0.6;
    qreal radius = qMin(CELL.width(), CELL.height()) / 2;
    path.addEllipse(QRectF(-radius, -radius, radius * 2, radius * 2));
    path.moveTo(0, -radius * crossSize);
    path.lineTo(0, radius * crossSize);
    path.moveTo(-radius * crossSize, 0);
    path.lineTo(radius * crossSize, 0);

    path.translate(point);

    ValencyPoint vPoint;
    vPoint.setPath(path);
    vPoint.setColor(QColor::fromRgba(0xff00ff00));

    return vPoint;
}

bool SortingBox::isCyclogramEndBranch(Command* cmd) const
{
    if (cmd->type() == ShapeTypes::TERMINATOR)
    {
        return true;
    }
    else if (cmd->type() == ShapeTypes::GO_TO_BRANCH)
    {
        return false; // do not search further
    }

    for (int i = 0, sz = cmd->nextCommands().size(); i < sz; ++i)
    {
        if (isCyclogramEndBranch(cmd->nextCommands()[i]))
        {
            return true;
        }
    }

    return false;
}

void SortingBox::addCommand(ShapeTypes type, const ValencyPoint& point)
{
    ShapeItem* owner = point.owner();
    Command* cmd = owner->command();

    // 1. Create new command
    Command* newCmd = mCurrentCyclogram->createCommand(type);
    if (!newCmd)
    {
        return;
    }

    int role = point.role();

    // 2. Update command tree connections
    cmd->insertCommand(newCmd, point.role());

    // 3. Add new command shape item to cyclogram view
    QPoint newCmdCell = owner->cell();
    newCmdCell.setY(newCmdCell.y() + 1);
    int TODO; // QUESTION/SWITCH commands cell will be shifted 1 column right
    ShapeItem* newItem = createCommandShape(newCmd, newCmdCell);

    // 4. Update commands positions below and to the right of the inserted command shape

    // 4.1 Find "expanded" elements in the insertion column, if it exist, just reduce the expanded element rect height
    ShapeItem* expandedItem = findExpandedItem(newItem);

    if (expandedItem)
    {
        // shift items between new inserted and found expanded item
        foreach (ShapeItem* item, mCommands)
        {
            if (item == newItem) // skip added item
            {
                continue;
            }

            if (item->cell().x() == newItem->cell().x())
            {
                if (item->cell().y() >= newItem->cell().y() && item->cell().y() < expandedItem->rect().top())
                {
                    updateItemGeometry(item, 0, 1, 1, 1);
                }
            }
        }

        // reduce expanded item size
        updateItemGeometry(expandedItem, 0, 0, 1, 0);

    }
    else // no expanded item in the column, shift items below the inserted in all columns
    {
        foreach (ShapeItem* item, mCommands)
        {
            if (item == newItem) // skip added item
            {
                continue;
            }

            if (item->cell().y() >= newItem->cell().y())
            {
                // shift items below the added one down by 1 cell in own column
                if (item->cell().x() == newItem->cell().x())
                {
                    updateItemGeometry(item, 0, 1, 1, 1);
                }
                else // expand the rect of the lowest item in all other columns
                {
                    if (item->cell().y() == (mDiagramSize.height() - 1))
                    {
                        updateItemGeometry(item, 0, 1, 0, 1);
                    }
                }
            }
        }

        //TODO on diagram size changed
        mDiagramSize.setHeight(mDiagramSize.height() + 1);
        drawSilhouette();
    }

    // 5. Update owner rect size recursively (for QUESTION/SWITCH-CASE command trees)
    int TODO2; // owner->parentTreeItem();
    QRect rect = owner->rect();
    owner->setRect(rect);
    update();
}

ShapeItem* SortingBox::findExpandedItem(ShapeItem* newItem) const
{
    ShapeItem* expandedItem = Q_NULLPTR;
    foreach (ShapeItem* item, mCommands)
    {
        if (item == newItem) // skip added item
        {
            continue;
        }

        // if element is in the new elements column and below it
        if (item->cell().x() == newItem->cell().x() && item->cell().y() >= newItem->cell().y())
        {
            // if it is an "expanded" item
            if (item->rect().height() > 1)
            {
                if (expandedItem)
                {
                    if (expandedItem->rect().top() > item->rect().top())
                    {
                        expandedItem = item;
                    }
                }
                else
                {
                    expandedItem = item;
                }
            }
        }
    }

    return expandedItem;
}

void SortingBox::updateItemGeometry(ShapeItem* item, int xShift, int yShift, int topShift, int bottomShift) const
{
    QPoint position = item->position();
    position.setX(position.x() + xShift * mItem.width());
    position.setY(position.y() + yShift * mItem.height());
    item->setPosition(position);

    QPoint cell = item->cell();
    cell.setX(cell.x() + xShift);
    cell.setY(cell.y() + yShift);
    item->setCell(cell);

    QRect rect = item->rect();
    rect.setTop(rect.top() + topShift);
    rect.setBottom(rect.bottom() + bottomShift);
    item->setRect(rect);

    if (topShift != bottomShift) // rect size changed, update path
    {
        item->setPath(createPath(item));
    }
}


void SortingBox::showEditDialog(ShapeItem *item)
{
    QDialog* dialog = mEditDialogs.value(item->command()->type(), Q_NULLPTR);

    if (dialog) // show custom dialog
    {
        switch (item->command()->type())
        {
        case ShapeTypes::DELAY:
            {
                CmdDelayEditDialog* d = qobject_cast<CmdDelayEditDialog*>(dialog);
                d->setCommand(qobject_cast<CmdDelay*>(item->command()));
            }
            break;
        default:
            break;
        }
    }
    else // show default dialog
    {
        mShapeEditDialog->setCommand(item->command());
        dialog = mShapeEditDialog;
    }

    dialog->exec();

    addText(item);
}
