#include <QtWidgets>
#include <QDebug>

#include "Headers/cyclogram_widget.h"
#include "Headers/shape_add_dialog.h"
#include "Headers/shape_edit_dialog.h"
#include "Headers/command_error_dialog.h"

#include "Headers/cyclogram.h"
#include "Headers/cell.h"
#include "Headers/command.h"
#include "Headers/commands/cmd_title.h"

#include "Headers/cmd_action_math_edit_dialog.h"
#include "Headers/commands/cmd_action_math.h"
#include "Headers/cmd_delay_edit_dialog.h"
#include "Headers/commands/cmd_delay.h"
#include "Headers/cmd_state_start_edit_dialog.h"
#include "Headers/commands/cmd_state_start.h"
#include "Headers/cmd_set_state_edit_dialog.h"
#include "Headers/commands/cmd_set_state.h"

CyclogramWidget::CyclogramWidget():
    mDiagramSize(0, 0),
    mCurrentCyclogram(Q_NULLPTR)
{
    mShapeAddDialog = new ShapeAddDialog(this);
    mShapeEditDialog = new ShapeEditDialog(this);

    mEditDialogs[DRAKON::DELAY] = new CmdDelayEditDialog(this);
    mEditDialogs[DRAKON::BRANCH_BEGIN] = new CmdStateStartEditDialog(this);
    mEditDialogs[DRAKON::GO_TO_BRANCH] = new CmdSetStateEditDialog(this);
    mEditDialogs[DRAKON::ACTION_MATH] = new CmdActionMathEditDialog(this);

    setMouseTracking(true);
    setBackgroundRole(QPalette::Base);
    mMovingItem = 0;
    mSelectedItem = 0;

    // Set cyclogram origin
    mOrigin.setX(ShapeItem::itemSize().width() / 4);
    mOrigin.setY(0);

    //setWindowTitle(tr("Tool Tips"));
    resize(1000, 600);

    setFocusPolicy(Qt::ClickFocus);
}

CyclogramWidget::~CyclogramWidget()
{
    clear(true);
}

void CyclogramWidget::clear(bool onDestroy)
{
    qDeleteAll(mCommands);
    qDeleteAll(mSihlouette);

    if (!onDestroy && mCurrentCyclogram)
    {
        disconnect(mCurrentCyclogram, SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
        disconnect(mCurrentCyclogram, SIGNAL(deleted(Command*)), this, SLOT(removeShape(Command*)));
    }

    mCurrentCyclogram = Q_NULLPTR;
}

bool CyclogramWidget::event(QEvent *event)
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

void CyclogramWidget::resizeEvent(QResizeEvent * /* event */)
{
}

void CyclogramWidget::keyPressEvent(QKeyEvent *event)
{
    bool processed = true;

    switch (event->key())
    {
    case Qt::Key_Delete:
        {
            if (mSelectedItem)
            {
                QString errorDesc;
                if (canBeDeleted(mSelectedItem, errorDesc))
                {
                    bool isBranch = mSelectedItem->command()->type() == DRAKON::BRANCH_BEGIN;
                    QString title = isBranch ? tr("Branch deletion") : tr("Command deletion");
                    QString text = tr("Are you sure that you want to delete ");
                    if (isBranch)
                    {
                        text += tr("entire branch with all its commands?");
                    }
                    else
                    {
                        text += tr("this command?");
                    }

                    if (QMessageBox::Yes == QMessageBox::warning(this, title, text, QMessageBox::Yes, QMessageBox::No))
                    {
                        ShapeItem* item = mSelectedItem;
                        clearSelection();
                        deleteCommand(item);
                    }
                }
                else
                {
                    QMessageBox::warning(this, tr("Command deletion"), tr("Command can not be deleted!\nReason: ") + errorDesc);
                }
            }
        }
        break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
        {
            if (mSelectedItem)
            {
                if ((mSelectedItem->command()->flags() & Command::Editable) != 0)
                {
                    showEditDialog(mSelectedItem->command());
                }
                else
                {
                    QMessageBox::warning(this, tr("Command editing"), tr("Command is not editable"));
                }
            }
        }
        break;

    default:
        processed = false;
        break;
    }

    if (!processed)
    {
        QWidget::keyPressEvent(event);
    }
}

void CyclogramWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawItems(mSihlouette, painter);
    drawItems(mCommands, painter);
}

void CyclogramWidget::drawItems(QList<ShapeItem*>& items, QPainter& painter)
{
    // draw commands shapes first
    foreach (ShapeItem* shapeItem, items)
    {
        painter.translate(shapeItem->position());
        painter.setBrush(shapeItem->color());
        painter.drawPath(shapeItem->path());
        painter.setBrush(QColor::fromRgba(0xff000000));
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

void CyclogramWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // no interaction while cyclogram running
        if (mCurrentCyclogram->state() == Cyclogram::RUNNING)
        {
            return;
        }

        // check valency point tap first
        ValencyPoint point;
        if (hasValencyPointAt(event->pos(), point))
        {
            clearSelection();

            mShapeAddDialog->setValencyPoint(point);
            mShapeAddDialog->exec();

            if (mShapeAddDialog->result() == QDialog::Accepted)
            {
                addCommand(mShapeAddDialog->shapeType(), point);
            }
        }
        else // if no valency point click, check command shape click
        {
            int index = commandAt(event->pos());
            if (index >= 0)
            {
                ShapeItem* clickedItem = mCommands[index];

                /*
                int TODO1; // split shapes on movable and selectable
                if (clickedItem->isMovable())
                {
                    int TODO2; // create command shape copy and drag it under cursor
                    mMovingItem = clickedItem;
                    mPreviousPosition = event->pos();
                    mCommands.move(index, mCommands.size() - 1);
                }
                */

                if (!mSelectedItem || mSelectedItem != clickedItem)
                {
                    clearSelection(false);
                    mSelectedItem = clickedItem;
                    mSelectedItem->setSelected(true);
                    update();
                }
            }
            else
            {
                clearSelection();
            }
        }
    }
}

void CyclogramWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    //remember that mousePressEvent will be called first!

    if (event->button() == Qt::LeftButton)
    {
        if (mCurrentCyclogram->state() == Cyclogram::RUNNING)
        {
            return; // to not edit cyclogram while it is executed
        }

        int index = commandAt(event->pos());
        if (index == -1)
        {
            return;
        }

        showEditDialog(mCommands[index]->command());
    }
}

void CyclogramWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mMovingItem && (event->buttons() & Qt::LeftButton))
    {
        moveItemTo(event->pos());
    }
}

void CyclogramWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (mMovingItem && event->button() == Qt::LeftButton)
    {
        moveItemTo(event->pos());
        mMovingItem = 0;
    }
}

int CyclogramWidget::commandAt(const QPoint &pos)
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

bool CyclogramWidget::hasValencyPointAt(const QPoint &pos, ValencyPoint& point)
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

void CyclogramWidget::moveItemTo(const QPoint &pos)
{
    QPoint offset = pos - mPreviousPosition;
    mMovingItem->setPosition(mMovingItem->position() + offset);
    mPreviousPosition = pos;
    update();
}

ShapeItem* CyclogramWidget::createCommandShape(Command* cmd, const QPoint& cell)
{
    QPoint pos(mOrigin.x() + cell.x() * ShapeItem::itemSize().width(), mOrigin.y() + cell.y() * ShapeItem::itemSize().height());

    int TODO; //update diagram size if adding is to the end of the diagram, REMOVE FROM THIS METHOD
    if (cell.x() + 1 > mDiagramSize.width())
    {
        mDiagramSize.setWidth(cell.x() + 1);
    }

    if (cell.y() + 1 > mDiagramSize.height())
    {
        mDiagramSize.setHeight(cell.y() + 1);
    }

    ShapeItem* shapeItem = new ShapeItem(this);
    shapeItem->setCommand(cmd);
    shapeItem->setToolTip(tr("Tooltip"));
    shapeItem->setPosition(pos);
    shapeItem->setCell(cell);
    shapeItem->setRect(QRect(cell.x(), cell.y(), 1, 1)); // by initial shape rect matches the occupied cell
    shapeItem->createPath();
    shapeItem->setValencyPoints(createValencyPoints(cmd));
    mCommands.append(shapeItem);

    connect(shapeItem, SIGNAL(changed()), this, SLOT(onNeedUpdate()));

    return shapeItem;
}

void CyclogramWidget::drawSilhouette()
{
    int TODO; // not good handling
    qDeleteAll(mSihlouette);
    mSihlouette.clear();

    QPoint bottomRight(INT_MIN, INT_MIN);
    QPoint bottomLeft(mOrigin.x(), mDiagramSize.height() * ShapeItem::itemSize().height());
    QPoint topLeft(mOrigin.x(), mOrigin.y() + ShapeItem::itemSize().height());
    QPoint topRight(INT_MIN, INT_MAX);

    QPainterPath silhouette;

    foreach (ShapeItem* shapeItem, mCommands)
    {
        if (shapeItem->command()->type() == DRAKON::GO_TO_BRANCH)
        {
            qreal x = shapeItem->position().x() + ShapeItem::itemSize().width() / 2;
            if (x > bottomRight.x())
            {
                qreal y = shapeItem->position().y() + ShapeItem::itemSize().height();
                bottomRight.setX(x);
                bottomRight.setY(y);
            }
        }
        else if (shapeItem->command()->type() == DRAKON::BRANCH_BEGIN)
        {
            qreal x = shapeItem->position().x() + ShapeItem::itemSize().width() / 2;
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

    ShapeItem* sihlouetteItem = new ShapeItem(this);
    sihlouetteItem->setPath(silhouette);
    sihlouetteItem->setPosition(QPoint(0, 0));
    sihlouetteItem->setColor(QColor::fromRgba(0x00ffffff));

    // draw arrow
    QPainterPath arrow;
    QPoint pos;
    pos.setX(topLeft.x() + ShapeItem::itemSize().width() / 2);
    pos.setY(topLeft.y());
    arrow.moveTo(pos);
    arrow.lineTo(QPoint(pos.x() - ShapeItem::cellSize().width(), pos.y() + ShapeItem::cellSize().height() / 4));
    arrow.lineTo(QPoint(pos.x() - ShapeItem::cellSize().width(), pos.y() - ShapeItem::cellSize().height() / 4));
    arrow.lineTo(pos);

    ShapeItem* arrowItem = new ShapeItem(this);
    arrowItem->setPath(arrow);
    arrowItem->setPosition(QPoint(0, 0));
    arrowItem->setColor(QColor::fromRgba(0xff000000));

    mSihlouette.push_back(sihlouetteItem);
    mSihlouette.push_back(arrowItem);
}

void CyclogramWidget::load(Cyclogram* cyclogram)
{
    clear();

    if (!cyclogram)
    {
        return;
    }

    Command* first = cyclogram->first();

    if (!first || first->type() != DRAKON::TERMINATOR)
    {
        return;
    }

    if (mCurrentCyclogram)
    {
        disconnect(mCurrentCyclogram, SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
        disconnect(mCurrentCyclogram, SIGNAL(deleted(Command*)), this, SLOT(removeShape(Command*)));
    }

    mCurrentCyclogram = cyclogram;
    connect(mCurrentCyclogram, SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
    connect(mCurrentCyclogram, SIGNAL(deleted(Command*)), this, SLOT(removeShape(Command*)));

    QPoint parentCell(0, 0);
    createCommandShape(first, parentCell);
    addChildCommands(first, parentCell);
    onNeedUpdate();
}

void CyclogramWidget::onCyclogramStateChanged(int state)
{
    clearSelection();
}

bool CyclogramWidget::canBeDeleted(ShapeItem* item, QString& error) const
{
    // item marked as "not deletable"
    if ((item->command()->flags() & Command::Deletable) == 0)
    {
        error = tr("Command is not deletable");
        return false;
    }

    // nothing can be deleted in running cyclogram
    if (mCurrentCyclogram->state() == Cyclogram::RUNNING)
    {
        error = tr("THIS TEXT SHOULD NEVER APPEAR ON THE SCREEN!");
        return false;
    }

    if (item->command()->type() == DRAKON::BRANCH_BEGIN)
    {
        // START and END branches never can be deleted
        Command* startBranch = mCurrentCyclogram->first();

        // check is start branch trying to delete
        if (startBranch->nextCommands()[0] == item->command())
        {
            error = tr("Start branch never can be deleted");
            return false;
        }

        Command* lastBranch = mCurrentCyclogram->last()->parentCommand();

        while (lastBranch->type() != DRAKON::BRANCH_BEGIN)
        {
            lastBranch = lastBranch->parentCommand();
        }

        // check is end branch trying to delete
        if (lastBranch == item->command())
        {
            error = tr("End branch never can be deleted");
            return false;
        }
    }

    if (mCurrentCyclogram->state() == Cyclogram::PAUSED)
    {
        // current running command can not be deleted
        if (item->command() == mCurrentCyclogram->current())
        {
            error = tr("Current running command can not be deleted");
            return false;
        }

        // branch of current command can not be deleted
        if (item->command()->type() == DRAKON::BRANCH_BEGIN)
        {
            Command* currentBranch = mCurrentCyclogram->current()->parentCommand();

            while (currentBranch->type() != DRAKON::BRANCH_BEGIN)
            {
                currentBranch = currentBranch->parentCommand();
            }

            if (currentBranch == item->command())
            {
                error = tr("Has running command in this branch");
                return false;
            }
        }
    }

    return true;
}

void CyclogramWidget::clearSelection(bool needUpdate)
{
    if (mSelectedItem)
    {
        mSelectedItem->setSelected(false);
        mSelectedItem = 0;
        if (needUpdate)
        {
            update();
        }
    }
}

void CyclogramWidget::addChildCommands(Command* parentCmd, const QPoint& parentCell)
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

        if (cmd->type() == DRAKON::BRANCH_BEGIN && parentCmd->type() == DRAKON::GO_TO_BRANCH) // not first branch
        {
            cell.setX(mDiagramSize.width());
            cell.setY(1);
        }

        createCommandShape(cmd, cell);

        if (cmd->type() == DRAKON::GO_TO_BRANCH)
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

bool CyclogramWidget::isBranchExist(Command* goToBranchCmd)
{
    foreach (ShapeItem* shape, mCommands)
    {
        Command* command = shape->command();
        if (command->type() == DRAKON::BRANCH_BEGIN && command->text() == goToBranchCmd->text())
        {
            return true;
        }
    }

    return false;
}

QList<ValencyPoint> CyclogramWidget::createValencyPoints(Command* cmd)
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

    DRAKON::IconType type = cmd->type();

    switch (type)
    {
    case DRAKON::BRANCH_BEGIN:
    case DRAKON::ACTION_MATH:
    case DRAKON::ACTION_MODULE:
    case DRAKON::DELAY:
        {
            ValencyPoint point = createPoint(QPointF(ShapeItem::itemSize().width() / 2, ShapeItem::itemSize().height()), 0);
            points.push_back(point);

            if (type == DRAKON::BRANCH_BEGIN && !isCyclogramEndBranch(cmd))
            {
                ValencyPoint point = createPoint(QPointF(ShapeItem::itemSize().width(), 0), 1);
                points.push_back(point);
            }
        }
        break;

    case DRAKON::QUESTION:
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

ValencyPoint CyclogramWidget::createPoint(const QPointF& point, int role)
{
    QPainterPath path;

    qreal crossSize = 0.6;
    qreal radius = qMin(ShapeItem::cellSize().width(), ShapeItem::cellSize().height()) / 2;
    path.addEllipse(QRectF(-radius, -radius, radius * 2, radius * 2));
    path.moveTo(0, -radius * crossSize);
    path.lineTo(0, radius * crossSize);
    path.moveTo(-radius * crossSize, 0);
    path.lineTo(radius * crossSize, 0);

    path.translate(point);

    ValencyPoint vPoint;
    vPoint.setPath(path);
    vPoint.setColor(QColor::fromRgba(0xff00ff00));
    vPoint.setRole(role);

    return vPoint;
}

bool CyclogramWidget::isCyclogramEndBranch(Command* cmd) const
{
    if (cmd->type() == DRAKON::TERMINATOR)
    {
        return true;
    }
    else if (cmd->type() == DRAKON::GO_TO_BRANCH)
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

ShapeItem* CyclogramWidget::addCommand(DRAKON::IconType type, const ValencyPoint& point)
{
    int role = point.role();
    if (type == DRAKON::BRANCH_BEGIN && role == 1) //TODO make enum for roles?
    {
        return addNewBranch(point.owner());
    }

    // 1. Create new command
    Command* newCmd = mCurrentCyclogram->createCommand(type);
    if (!newCmd)
    {
        return Q_NULLPTR;
    }

    ShapeItem* owner = point.owner();
    Command* cmd = owner->command();

    // 2. Update command tree connections
    if (cmd->type() == DRAKON::BRANCH_BEGIN && newCmd->type() == DRAKON::GO_TO_BRANCH) // new branch creation
    {
        cmd->addCommand(newCmd, role);
    }
    else
    {
        cmd->insertCommand(newCmd, role);
    }

    // 3. Add new command shape item to cyclogram view
    QPoint newCmdCell = owner->cell();
    // create new chape below the points' owner
    newCmdCell.setY(newCmdCell.y() + 1);

    int TODO; // QUESTION/SWITCH commands cell will be shifted 1 column right
    /* Мысли вслух по добавлению QUESTION
     * 1. При редактировании QUESTIONа можно указать следующую команду для ветки вправо (любая команда слева от question)
     * 2. Можно указать ТОЛЬКО на команду, расположенную в столбце слева (вероятно)
     * 3. QUESION, если вставляется в самый правый столбец бранча, увеличивает его ширину на 1
    */
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
        if (owner->command()->type() == DRAKON::BRANCH_BEGIN && newItem->command()->type() == DRAKON::GO_TO_BRANCH)
        {// new branch creation

            int y = newItem->cell().y();
            int h = mDiagramSize.height();
            int shift = h - y - 1;
            updateItemGeometry(newItem, 0, shift, 0, shift);
        }
        else // new item in branch adding
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
            onNeedUpdate();
        }
    }

    // 5. Update owner rect size recursively (for QUESTION/SWITCH-CASE command trees)
    int TODO2; // owner->parentTreeItem();
    QRect rect = owner->rect();
    owner->setRect(rect);
    update();

    return newItem;
}

void CyclogramWidget::deleteCommand(ShapeItem* item)
{
    bool deleteShape = true;

    int TODO; // first try to do simple command deletion (linear comand inside some branch)

    switch (item->command()->type())
    {
    case DRAKON::ACTION_MATH:
    case DRAKON::ACTION_MODULE:
    case DRAKON::DELAY:
        {
            int TODO3; // this is only for one-column branches! QUESTION/CASE will require some refactor
            ShapeItem* expandedItem = findExpandedItem(item);

            if (expandedItem)
            {
                // not the longest branch ->
                // 1. Shift up items below that need to be deleted (with except of expandedItem)
                // 2. Expand the rect of expandedItem by 1 to top
                foreach (ShapeItem* it, mCommands)
                {
                    if (it == item)
                    {
                        continue;
                    }

                    // if element is in the deleted elements column and below it
                    if (it->cell().x() == item->cell().x() && it->cell().y() >= item->cell().y())
                    {
                        int h = item->rect().height();
                        if (it != expandedItem)
                        {
                            updateItemGeometry(it, 0, -h, -h, -h);
                        }
                        else
                        {
                            updateItemGeometry(it, 0, 0, -h, 0);
                        }
                    }
                }
            }
            else
            {
                int TODO4; // this is only for one-column branches system! QUESTION/CASE will require some refactor

                bool isLongestColumn = true;
                QList<ShapeItem*> otherGoToBranchCommands;
                ShapeItem* ownGoToBranchItem = Q_NULLPTR;

                foreach (ShapeItem* it, mCommands)
                {
                    // it is GO_TO_BRANCH or cyclogram end TERMINATOR
                    if (it->command()->type() == DRAKON::GO_TO_BRANCH || (it->command()->type() == DRAKON::TERMINATOR && it->cell().y() > 0))
                    {
                        if (it->cell().x() != item->cell().x())
                        {
                            otherGoToBranchCommands.push_back(it);

                            if (isLongestColumn && it->rect().height() == 1)
                            {
                                isLongestColumn = false;
                            }
                        }
                        else
                        {
                            ownGoToBranchItem = it;
                        }
                    }
                }

                if (isLongestColumn)
                {
                    // reduce GO_TO_BRANCH commands rect in other columns

                    // Shift up ALL items below that need to be deleted (including GO_TO_BRANCH command)
                    foreach (ShapeItem* it, mCommands)
                    {
                        if (it == item)
                        {
                            continue;
                        }

                        // if element is in the deleted elements column and below it
                        if (it->cell().x() == item->cell().x() && it->cell().y() >= item->cell().y())
                        {
                            int h = item->rect().height();
                            updateItemGeometry(it, 0, -h, -h, -h);
                        }
                    }

                    // reduce size of the terminating commands in other columns

                    int h = item->rect().height();
                    foreach (ShapeItem* it, otherGoToBranchCommands)
                    {
                        updateItemGeometry(it, 0, -h, 0, -h);
                    }

                    // diagram size reduced
                    mDiagramSize.setHeight(mDiagramSize.height() - h);
                }
                else
                {
                    // TODO almost copypaste of if expandedItem
                    // just delete command and expand rect of the columns GO_TO_BRANCH item
                    foreach (ShapeItem* it, mCommands)
                    {
                        if (it == item)
                        {
                            continue;
                        }

                        // if element is in the deleted elements column and below it
                        if (it->cell().x() == item->cell().x() && it->cell().y() >= item->cell().y())
                        {
                            int h = item->rect().height();
                            if (it != ownGoToBranchItem)
                            {
                                updateItemGeometry(it, 0, -h, -h, -h);
                            }
                            else
                            {
                                updateItemGeometry(it, 0, 0, -h, 0);
                            }
                        }
                    }
                }
            }

            int TODO2; // possibly update parent QUESTION command rect here
        }
        break;

    case DRAKON::BRANCH_BEGIN:
        {
            deleteBranch(item);
            deleteShape = false;
        }
        break;

    default:
        {
            qDebug("Try to delete non-deleatable");
            deleteShape = false;
        }
        break;
    }

    if (deleteShape)
    {
        mCurrentCyclogram->deleteCommand(item->command()); // shape will be deleted by the signal
    }
}

void CyclogramWidget::removeShape(Command* command)
{
    for (int i = 0, sz = mCommands.size(); i < sz; ++i)
    {
        if (mCommands[i]->command() == command)
        {
            ShapeItem* tmp = mCommands.takeAt(i);
            tmp->deleteLater();
            break;
        }
    }

    onNeedUpdate();
}

void CyclogramWidget::onNeedUpdate()
{
    drawSilhouette();
    update();
}

void CyclogramWidget::deleteBranch(ShapeItem* item)
{
    int TODO; // this only working for one-column branches, QUESTION/CASE command adding will require some refactoring

    bool isLongestColumn = true;
    QList<ShapeItem*> otherGoToBranchCommands;
    ShapeItem* ownGoToBranchItem = Q_NULLPTR;

    // find out is deleting branch the longest one
    foreach (ShapeItem* it, mCommands)
    {
        // it is GO_TO_BRANCH or cyclogram end TERMINATOR
        if (it->command()->type() == DRAKON::GO_TO_BRANCH || (it->command()->type() == DRAKON::TERMINATOR && it->cell().y() > 0))
        {
            if (it->cell().x() != item->cell().x())
            {
                otherGoToBranchCommands.push_back(it);

                if (isLongestColumn && it->rect().height() == 1)
                {
                    isLongestColumn = false;
                }
            }
            else
            {
                ownGoToBranchItem = it;
            }
        }
    }

    int yOffset = 0;

    if (isLongestColumn) // the longes one
    {
        // reduce GO_TO_BRANCH commands rect in other columns by the second longest branch expansion size
        // find set second branch by length (it has the lowest height)
        int minHeight = INT_MAX;
        ShapeItem* secondMinBranch = Q_NULLPTR;

        foreach (ShapeItem* it, otherGoToBranchCommands)
        {
            if (it->rect().height() < minHeight)
            {
                minHeight = it->rect().height();
                secondMinBranch = it;
            }
        }

        yOffset = minHeight - 1;
    }

    // shrink other columns size
    // update links to the deleted branch
    foreach (ShapeItem* it, otherGoToBranchCommands)
    {
        updateItemGeometry(it, 0, -yOffset, 0, -yOffset);

        // if other GO_TO_BRANCH commands refer to deleting branch, set error status on them
        const QList<Command*>& next = it->command()->nextCommands();
        if (!next.empty() && next[0] == item->command()) // first check is for TERMINATOR, cause it never has any next commands
        {
            it->command()->replaceCommand(Q_NULLPTR);
        }
    }

    // diagram size reduced
    mDiagramSize.setHeight(mDiagramSize.height() - yOffset);

    int w = item->rect().width();

    // Anyway shift to the left all branches right to deleting one
    foreach (ShapeItem* it, mCommands)
    {
        if (it == item)
        {
            continue;
        }

        if (it->cell().x() > item->cell().x())
        {
            updateItemGeometry(it, -w, 0, 0, 0);
        }
    }

    mDiagramSize.setWidth(mDiagramSize.height() - w);

    // kill all shapes and commands, belonging to deleting branch
    ownGoToBranchItem->command()->replaceCommand(Q_NULLPTR); // to block further tree deletion
    mCurrentCyclogram->deleteCommand(item->command(), true);
    int TODO5; // make control over command by shape (delete shape = delete command)
}

ShapeItem* CyclogramWidget::addNewBranch(ShapeItem* item)
{
    // create new branch to the right of the item command tree
    QPoint newCmdCell = item->cell();
    ShapeItem* nextBranch = findNextBranch(newCmdCell);
    newCmdCell.setX(nextBranch->cell().x());

    // 1. Create and add BRANCH_BEGIN item
    Command* newCmd = mCurrentCyclogram->createCommand(DRAKON::BRANCH_BEGIN);
    if (!newCmd)
    {
        return Q_NULLPTR;
    }

    QString name = generateBranchName();

    // do not create links to new branch
    // Update commands positions to the right of the inserted branch
    int xNext = nextBranch->cell().x();
    foreach (ShapeItem* it, mCommands)
    {
        if (it->cell().x() >= xNext)
        {
            updateItemGeometry(it, 1, 0, 0, 0);
        }
    }

    ShapeItem* newBranchItem = createCommandShape(newCmd, newCmdCell);

    // 2. Create and add GO_TO_BRANCH item to the created branch
    ShapeItem* goToBranchItem = addCommand(DRAKON::GO_TO_BRANCH, newBranchItem->valencyPoint(0));

    // by default new branch is linked to itself
    goToBranchItem->command()->addCommand(newBranchItem->command());

    //generate unique branch name
    CmdStateStart* cmd = qobject_cast<CmdStateStart*>(newCmd);
    cmd->setText(name);

    mDiagramSize.setWidth(mDiagramSize.width() + 1);
    onNeedUpdate();

    return newBranchItem;
}

QString CyclogramWidget::generateBranchName() const
{
    QList<ShapeItem*> existingBranches;

    foreach (ShapeItem* it, mCommands)
    {
        if (it->command()->type() == DRAKON::BRANCH_BEGIN)
        {
            existingBranches.push_back(it);
        }
    }

    QString prefix = tr("New Branch");
    QString name = prefix;
    bool nameGenerated = false;
    int i = 1;
    while (!nameGenerated)
    {
        bool exist = false;
        foreach (ShapeItem* it, existingBranches)
        {
            if (it->command()->text() == name)
            {
                exist = true;
                break;
            }
        }

        nameGenerated = !exist;

        if (!nameGenerated)
        {
            name = prefix + QString(" ") + QString::number(i);
            ++i;
        }
    }

    return name;
}

ShapeItem* CyclogramWidget::findExpandedItem(ShapeItem* newItem) const
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

ShapeItem* CyclogramWidget::findNextBranch(const QPoint& cell) const
{
    ShapeItem* item = Q_NULLPTR;
    int column = INT_MAX;

    foreach (ShapeItem* it, mCommands)
    {
        if (it->command()->type() == DRAKON::BRANCH_BEGIN)
        {
            int x = it->cell().x();
            if (x > cell.x() && x < column)
            {
                column = x;
                item = it;
            }
        }
    }

    return item;
}

void CyclogramWidget::updateItemGeometry(ShapeItem* item, int xShift, int yShift, int topShift, int bottomShift) const
{
    QPoint position = item->position();
    position.setX(position.x() + xShift * ShapeItem::itemSize().width());
    position.setY(position.y() + yShift * ShapeItem::itemSize().height());
    item->setPosition(position);

    QPoint cell = item->cell();
    cell.setX(cell.x() + xShift);
    cell.setY(cell.y() + yShift);
    item->setCell(cell);

    QRect rect = item->rect();
    rect.setTop(rect.top() + topShift);
    rect.setBottom(rect.bottom() + bottomShift);
    rect.setLeft(rect.left() + xShift);
    rect.setRight(rect.right() + xShift);
    item->setRect(rect);

    if (topShift != bottomShift) // rect size changed, update path
    {
        item->createPath();
    }
}

void CyclogramWidget::showEditDialog(Command *command)
{
    QDialog* dialog = mEditDialogs.value(command->type(), Q_NULLPTR);

    if (dialog) // show custom dialog
    {
        switch (command->type())
        {
        case DRAKON::DELAY:
            {
                CmdDelayEditDialog* d = qobject_cast<CmdDelayEditDialog*>(dialog);
                d->setCommand(qobject_cast<CmdDelay*>(command));
            }
            break;

        case DRAKON::ACTION_MATH:
            {
                CmdActionMathEditDialog* d = qobject_cast<CmdActionMathEditDialog*>(dialog);
                d->setCommand(qobject_cast<CmdActionMath*>(command));
            }
            break;

        case DRAKON::BRANCH_BEGIN:
            {
                CmdStateStartEditDialog* d = qobject_cast<CmdStateStartEditDialog*>(dialog);
                QList<Command*> commands;
                foreach (ShapeItem* it, mCommands)
                {
                    if (it->command()->type() == DRAKON::BRANCH_BEGIN && it->command() != command)
                    {
                        commands.push_back(it->command());
                    }
                }

                d->setCommands(qobject_cast<CmdStateStart*>(command), commands);
            }
            break;

        case DRAKON::GO_TO_BRANCH:
            {
                CmdSetStateEditDialog* d = qobject_cast<CmdSetStateEditDialog*>(dialog);
                QList<Command*> commands;
                foreach (ShapeItem* it, mCommands)
                {
                    if (it->command()->type() == DRAKON::BRANCH_BEGIN)
                    {
                        commands.push_back(it->command());
                    }
                }

                d->setCommands(qobject_cast<CmdSetState*>(command), commands);
            }
            break;

        default:
            break;
        }
    }
    else // show default dialog
    {
        mShapeEditDialog->setCommand(command);
        dialog = mShapeEditDialog;
    }

    dialog->exec();
}

void CyclogramWidget::showValidationError(Command* cmd)
{
    CommandErrorDialog* dialog = new CommandErrorDialog(this);
    dialog->setText(cmd->text());
    dialog->exec();

    if (dialog->result() == QDialog::Accepted)
    {
        showEditDialog(cmd);
    }

    dialog->deleteLater();
}
