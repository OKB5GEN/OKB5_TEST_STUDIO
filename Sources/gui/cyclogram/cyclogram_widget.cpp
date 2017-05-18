#include <QtWidgets>
#include <QDebug>
#include <QFile>

#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/gui/cyclogram/dialogs/shape_add_dialog.h"
#include "Headers/gui/cyclogram/dialogs/shape_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/command_error_dialog.h"

#include "Headers/logic/cyclogram.h"
#include "Headers/logic/command.h"
#include "Headers/logic/commands/cmd_title.h"

#include "Headers/logic/commands/cmd_action_math.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/logic/commands/cmd_delay.h"
#include "Headers/logic/commands/cmd_output.h"
#include "Headers/logic/commands/cmd_parallel_process.h"
#include "Headers/logic/commands/cmd_state_start.h"
#include "Headers/logic/commands/cmd_set_state.h"
#include "Headers/logic/commands/cmd_question.h"
#include "Headers/logic/commands/cmd_sub_program.h"

#include "Headers/gui/cyclogram/dialogs/cmd_action_math_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_action_module_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_delay_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_state_start_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_set_state_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_question_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_subprogram_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_terminator_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_output_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_parallel_process_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/subprogram_dialog.h"
#include "Headers/gui/tools/monitor_auto.h"

#include "Headers/gui/cyclogram/shape_item.h"

#include "Headers/logger/Logger.h"

#include "Headers/file_reader.h"

namespace
{
    static const qreal MAX_SCALE = 1.0;
    static const qreal DEFAULT_SCALE = 1.0;
    static const qreal MIN_SCALE = 0.2;
    static const qreal SCALE_CHANGE_STEP = 0.025;
}

CyclogramWidget::CyclogramWidget(QWidget* parent):
    QWidget(parent),
    mSihlouetteLine(Q_NULLPTR),
    mSihlouetteArrow(Q_NULLPTR),
    mCurSubprogram(Q_NULLPTR),
    mDialogParent(Q_NULLPTR),
    mScale(DEFAULT_SCALE)
{
    setMouseTracking(true);
    setBackgroundRole(QPalette::Base);

    QPalette pal(QGuiApplication::palette());
    pal.setColor(QPalette::Base, QColor::fromRgba(0xffdfdfdf));
    setPalette(pal);

    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    mMovingItem = 0;
    mSelectedItem = 0;
    mRootShape = 0;

    setFocusPolicy(Qt::ClickFocus);
}

CyclogramWidget::~CyclogramWidget()
{
    clear(true);
}

void CyclogramWidget::clear(bool onDestroy)
{
    clearSelection();

    qDeleteAll(mCommands);
    mCommands.clear();

    if (!onDestroy && mCurrentCyclogram.data())
    {
        disconnect(mCurrentCyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
        disconnect(mCurrentCyclogram.data(), SIGNAL(deleted(Command*)), this, SLOT(removeShape(Command*)));
    }

    mCurrentCyclogram.clear();
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

void CyclogramWidget::wheelEvent(QWheelEvent *event)
{
    if (QApplication::keyboardModifiers() && Qt::ControlModifier)
    {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;

        mScale += numSteps * SCALE_CHANGE_STEP;

        if (mScale > MAX_SCALE)
        {
            mScale = MAX_SCALE;
        }

        if (mScale < MIN_SCALE)
        {
            mScale = MIN_SCALE;
        }

        event->accept();
        onNeedUpdate();
    }
}

void CyclogramWidget::deleteSelectedItem()
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

void CyclogramWidget::keyPressEvent(QKeyEvent *event)
{
    bool processed = true;

    switch (event->key())
    {
    case Qt::Key_Delete:
        {
            deleteSelectedItem();
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
    painter.scale(mScale, mScale);

    QList<ShapeItem*> sihlouetteItems;
    sihlouetteItems.push_back(mSihlouetteLine);
    sihlouetteItems.push_back(mSihlouetteArrow);

    drawItems(sihlouetteItems, painter);
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
        painter.setBrush(shapeItem->additionalColor());
        painter.drawPath(shapeItem->additionalPath());
        painter.setBrush(QColor::fromRgba(0xff000000));
        painter.drawPath(shapeItem->arrowPath());
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
        if (mCurrentCyclogram.lock()->state() == Cyclogram::RUNNING)
        {
            return;
        }

        // check valency point tap first
        ValencyPoint point;
        if (hasValencyPointAt(event->pos(), point))
        {
            clearSelection();

            ShapeAddDialog* dialog = new ShapeAddDialog(this);

            dialog->setValencyPoint(point);
            dialog->exec();

            if (dialog->result() == QDialog::Accepted)
            {
                addCommand(dialog->shapeType(), point, dialog->param());
            }

            dialog->deleteLater();
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
    else if (event->button() == Qt::RightButton)
    {
        int index = commandAt(event->pos());
        if (index >= 0)
        {
            ShapeItem* clickedItem = mCommands[index];

            clearSelection(false);
            mSelectedItem = clickedItem;
            mSelectedItem->setSelected(true);
            update();

            if (mSelectedItem->command()->type() == DRAKON::SUBPROGRAM)
            {
                mCurSubprogram = qobject_cast<CmdSubProgram*>(mSelectedItem->command());
                QMenu *menu = new QMenu(this);
                menu->addAction(tr("Show subprogram"), this, SLOT(showSubprogramWidget()));
                menu->addAction(tr("Show subprogram chart"), this, SLOT(showSubprogramChart()));
                menu->exec(mapToGlobal(event->pos()));
                menu->deleteLater();
            }
        }
        else
        {
            clearSelection();
        }
    }
}

void CyclogramWidget::showSubprogramWidget()
{
    if (!mCurSubprogram)
    {
        LOG_WARNING(QString("Subprogram not set 1"));
        return;
    }

    if (!mCurSubprogram->loaded())
    {
        LOG_ERROR(QString("Subprogram configuration error 1"));
        return;
    }

    Q_ASSERT(mDialogParent);
    SubProgramDialog* subProgramDialog = new SubProgramDialog(mCurSubprogram, mDialogParent);
    QString title = updateWindowTitle(subProgramDialog);
    subProgramDialog->cyclogramWidget()->setParentTitle(title);
    subProgramDialog->show();
}

void CyclogramWidget::showSubprogramChart()
{
    if (!mCurSubprogram)
    {
        LOG_WARNING(QString("Subprogram not set 3"));
        return;
    }

    if (!mCurSubprogram->loaded())
    {
        LOG_ERROR(QString("Subprogram configuration error 3"));
        return;
    }

    Q_ASSERT(mDialogParent);
    MonitorAuto* dialog = new MonitorAuto(mDialogParent);
    updateWindowTitle(dialog);

    dialog->setCyclogram(mCurSubprogram->cyclogram());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

QString CyclogramWidget::updateWindowTitle(QWidget* dialog)
{
    QString title;
    if (!mCurSubprogram)
    {
        LOG_WARNING(QString("Subprogram not set 2"));
        return title;
    }

    if (!mCurSubprogram->loaded())
    {
        LOG_ERROR(QString("Subprogram configuration error 2"));
        return title;
    }

    if (mCurrentCyclogram.lock()->isMainCyclogram())
    {
        title = mCurSubprogram->text();
    }
    else
    {
        title = mParentTitle;
        title += QString(" -> ");
        title += mCurSubprogram->text();
    }

    dialog->setWindowTitle(title);
}

void CyclogramWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    //TODO remember that mousePressEvent will be called first!
    if (event->button() == Qt::LeftButton)
    {
        if (mCurrentCyclogram.lock()->state() == Cyclogram::RUNNING)
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
        if (item->path().contains((pos / mScale) - item->position()))
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
            if (points[j].path().contains((pos / mScale) - mCommands[i]->position()))
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
    //mMovingItem->setPosition(mMovingItem->position() + offset); //TODO item moving disabled
    mPreviousPosition = pos;
    update();
}

ShapeItem* CyclogramWidget::addShape(Command* cmd, const QPoint& cell, ShapeItem* parentShape)
{
    ShapeItem* shapeItem = new ShapeItem(this);
    shapeItem->setCommand(cmd);
    shapeItem->setToolTip(tr("Tooltip"));
    shapeItem->setCell(cell);
    shapeItem->setParentShape(parentShape);
    shapeItem->setRect(QRect(cell.x(), cell.y(), 1, 1), false); // by initial shape rect matches the occupied cell

    shapeItem->updateFlags();

    mCommands.append(shapeItem);
    connect(shapeItem, SIGNAL(changed()), this, SLOT(onNeedUpdate()));
    connect(shapeItem, SIGNAL(needToDelete(ShapeItem*)), this, SLOT(onNeedToDelete(ShapeItem*)));

    return shapeItem;
}

void CyclogramWidget::drawSilhouette()
{
    QPoint bottomRight(INT_MIN, INT_MIN);
    QPoint bottomLeft(ShapeItem::origin().x(), mRootShape->rect().height() * ShapeItem::itemSize().height());
    QPoint topLeft(ShapeItem::origin().x(), ShapeItem::origin().y() + ShapeItem::itemSize().height());
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

    if (!mSihlouetteLine)
    {
        mSihlouetteLine = new ShapeItem(this);
        mSihlouetteLine->setColor(QColor::fromRgba(0x00ffffff));
    }

    mSihlouetteLine->setPath(silhouette);

    if (!mSihlouetteArrow)
    {
        // draw arrow
        QPainterPath arrow;
        QPoint pos;
        pos.setX(topLeft.x() + ShapeItem::itemSize().width() / 2);
        pos.setY(topLeft.y());
        arrow.moveTo(pos);
        arrow.lineTo(QPoint(pos.x() - ShapeItem::cellSize().width(), pos.y() + ShapeItem::cellSize().height() / 4));
        arrow.lineTo(QPoint(pos.x() - ShapeItem::cellSize().width(), pos.y() - ShapeItem::cellSize().height() / 4));
        arrow.lineTo(pos);

        mSihlouetteArrow = new ShapeItem(this);
        mSihlouetteArrow->setPath(arrow);
        mSihlouetteArrow->setColor(QColor::fromRgba(0xff000000));
    }
}

void CyclogramWidget::load(QSharedPointer<Cyclogram> cyclogram)
{
    mScale = DEFAULT_SCALE;

    clear();

    if (cyclogram.isNull())
    {
        return;
    }

    Command* first = cyclogram->first();

    if (!first || first->type() != DRAKON::TERMINATOR)
    {
        return;
    }

    if (mCurrentCyclogram.data())
    {
        disconnect(mCurrentCyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
        disconnect(mCurrentCyclogram.data(), SIGNAL(deleted(Command*)), this, SLOT(removeShape(Command*)));
    }

    mCurrentCyclogram = cyclogram;
    connect(cyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
    connect(cyclogram.data(), SIGNAL(deleted(Command*)), this, SLOT(removeShape(Command*)));

    ShapeItem* title = addShape(first, QPoint(0, 0), 0);
    mRootShape = title;
    drawCyclogram(title);

    mRootShape->adjust();

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
    if (mCurrentCyclogram.lock()->state() == Cyclogram::RUNNING)
    {
        error = tr("THIS TEXT SHOULD NEVER APPEAR ON THE SCREEN!");
        return false;
    }

    if (item->command()->type() == DRAKON::BRANCH_BEGIN)
    {
        // START and END branches never can be deleted
        Command* startBranch = mCurrentCyclogram.lock()->first();

        // check is start branch trying to delete
        if (startBranch->nextCommand() == item->command())
        {
            error = tr("Start branch never can be deleted");
            return false;
        }

        const ShapeItem* lastBranch = findBranch(mCurrentCyclogram.lock()->last());

        // check is end branch trying to delete
        if (lastBranch->command() == item->command())
        {
            error = tr("End branch never can be deleted");
            return false;
        }
    }
#ifdef ENABLE_CYCLOGRAM_PAUSE
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
            const ShapeItem* currentBranch = findBranch(mCurrentCyclogram->current());

            if (currentBranch->command() == item->command())
            {
                error = tr("Has running command in this branch");
                return false;
            }
        }
    }
#endif

    return true;
}

const ShapeItem* CyclogramWidget::findBranch(const Command* command) const
{
    if (!command)
    {
        return Q_NULLPTR;
    }

    foreach (const ShapeItem* item, mCommands)
    {
        if (item->command() == command)
        {
            while (command->type() != DRAKON::BRANCH_BEGIN)
            {
                item = item->parentShape();
                command = item->command();
            }

            return item;
        }
    }

    return Q_NULLPTR;
}

void CyclogramWidget::clearSelection(bool needUpdate)
{
    mCurSubprogram = Q_NULLPTR;

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

void CyclogramWidget::onNeedToDelete(ShapeItem* shape)
{
    mCurrentCyclogram.lock()->deleteCommand(shape->command());
}

void CyclogramWidget::onNeedUpdate()
{
    int W = ShapeItem::itemSize().width();
    int H = ShapeItem::itemSize().height();
    int w = ShapeItem::cellSize().width();
    int h = ShapeItem::cellSize().height();

    int width = W * mRootShape->rect().width() + ShapeItem::origin().x() + w;
    int height = H * mRootShape->rect().height() + ShapeItem::origin().y() + h;

    width *= mScale;
    height *= mScale;

    resize(width, height);

    drawSilhouette();
    update();
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

void CyclogramWidget::showEditDialog(Command *command)
{
    QDialog* dialog = Q_NULLPTR;

    switch (command->type())
    {
    case DRAKON::DELAY:
        {
            CmdDelayEditDialog* d = new CmdDelayEditDialog(this);
            d->setCommand(qobject_cast<CmdDelay*>(command));
            dialog = d;
        }
        break;

    case DRAKON::ACTION_MATH:
        {
            CmdActionMathEditDialog* d = new CmdActionMathEditDialog(this);
            d->setCommand(qobject_cast<CmdActionMath*>(command));
            dialog = d;
        }
        break;

    case DRAKON::ACTION_MODULE:
        {
            CmdActionModuleEditDialog* d = new CmdActionModuleEditDialog(this);
            d->setCommand(qobject_cast<CmdActionModule*>(command));
            dialog = d;
        }
        break;

    case DRAKON::BRANCH_BEGIN:
        {
            CmdStateStartEditDialog* d = new CmdStateStartEditDialog(this);
            QList<Command*> commands;
            foreach (ShapeItem* it, mCommands)
            {
                if (it->command()->type() == DRAKON::BRANCH_BEGIN && it->command() != command)
                {
                    commands.push_back(it->command());
                }
            }

            d->setCommands(qobject_cast<CmdStateStart*>(command), commands);
            dialog = d;
        }
        break;

    case DRAKON::GO_TO_BRANCH:
        {
            CmdSetStateEditDialog* d = new CmdSetStateEditDialog(this);
            QList<Command*> commands;
            foreach (ShapeItem* it, mCommands)
            {
                if (it->command()->type() == DRAKON::BRANCH_BEGIN)
                {
                    commands.push_back(it->command());
                }
            }

            d->setCommands(qobject_cast<CmdSetState*>(command), commands);
            dialog = d;
        }
        break;

    case DRAKON::QUESTION:
        {
            CmdQuestionEditDialog* d = new CmdQuestionEditDialog(this);
            d->setCommand(qobject_cast<CmdQuestion*>(command));
            dialog = d;
        }
        break;

    case DRAKON::SUBPROGRAM:
        {
            CmdSubProgramEditDialog* d = new CmdSubProgramEditDialog(this);
            d->setCommand(qobject_cast<CmdSubProgram*>(command), mCurrentCyclogram.lock());
            dialog = d;
        }
        break;

    case DRAKON::TERMINATOR:
        {
            CmdTerminatorEditDialog* d = new CmdTerminatorEditDialog(this);
            d->setCommand(qobject_cast<CmdTitle*>(command));
            dialog = d;
        }
        break;

    case DRAKON::OUTPUT:
        {
            CmdOutputEditDialog* d = new CmdOutputEditDialog(this);
            d->setCommand(qobject_cast<CmdOutput*>(command));
            dialog = d;
        }
        break;

    case DRAKON::PARALLEL_PROCESS:
        {
            CmdParallelProcessEditDialog* d = new CmdParallelProcessEditDialog(this);
            d->setCommand(qobject_cast<CmdParallelProcess*>(command));
            dialog = d;
        }
        break;

    default:
        break;
    }

    if (!dialog) // show default dialog
    {
        LOG_WARNING(QString("Default shape edit dualog launched"));
        ShapeEditDialog* shapeEditDialog = new ShapeEditDialog(this);
        shapeEditDialog->setCommand(command);
        dialog = shapeEditDialog;
    }

    dialog->exec();
    dialog->deleteLater();
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

void CyclogramWidget::drawCyclogram(ShapeItem* item)
{
    Command* cmd = item->command();

    if (cmd->type() != DRAKON::TERMINATOR || cmd->nextCommands().empty())
    {
        LOG_WARNING(QString("Not cyclogram start terminator"));
        return;
    }

    QList<Command*> branches;
    mCurrentCyclogram.lock()->getBranches(branches);

    int maxHeight = -1;
    int width = 0;

    QList<ShapeItem*> branchesShapes;

    // draw all branches
    foreach (Command* it, branches)
    {
        QPoint cell;
        cell.setX(width);
        cell.setY(item->cell().y() + 1);

        ShapeItem* shape = addShape(it, cell, item);
        QList<Command*> stopList;
        drawChildren(shape, stopList, true);
        branchesShapes.push_back(shape);

        QRect rect = shape->rect();
        width += rect.width();

        if (rect.height() > maxHeight)
        {
            maxHeight = rect.height();
        }
    }

    // adjust branches height
    foreach (ShapeItem* it, branchesShapes)
    {
        QRect rect = it->rect();
        if (rect.height() < maxHeight)
        {
            rect.setBottom(rect.bottom() + maxHeight - rect.height());
            it->setRect(rect, true);
        }

        item->addChildShape(it);
    }

    QRect rect;
    rect.setRight(width - 1);
    rect.setBottom(maxHeight);
    item->setRect(rect, false);
}

void CyclogramWidget::drawChildren(ShapeItem* item, const QList<Command*>& stopDrawingCommands, bool drawGoToBranch)
{
    Command* cmd = item->command();

    if (cmd->nextCommands().empty())
    {
        return; // command has no valency points
    }

    if (cmd->nextCommands().size() == 1)
    {
        if (!cmd->nextCommand())
        {
            return; // empty valency point or branch end
        }

        Command* nextCmd = cmd->nextCommand();

        if (nextCmd->type() == DRAKON::GO_TO_BRANCH && !drawGoToBranch)
        {
            return;
        }

        if (stopDrawingCommands.contains(nextCmd))
        {
            //QString msg = QString("Stopping draw chidren: Cmd:%1 Next:%2 ").arg(cmd->text()).arg(nextCmd->text());
            //LOG_DEBUG(msg);
            return;
        }

        QPoint cell = item->cell();
        cell.setY(cell.y() + 1);

        ShapeItem* shape = addShape(nextCmd, cell, item);
        item->setChildShape(shape, ValencyPoint::Down);

        if (nextCmd->type() != DRAKON::GO_TO_BRANCH)
        {
            drawChildren(shape, stopDrawingCommands, drawGoToBranch);
        }

        QRect rect = item->rect();
        rect.setBottom(rect.bottom() + shape->rect().height());
        rect.setRight(rect.right() + shape->rect().width() - rect.width());
        item->setRect(rect, false);

    }
    else if (cmd->nextCommands().size() == 3)
    {
        if (cmd->type() == DRAKON::QUESTION)
        {
            CmdQuestion* questionCmd = qobject_cast<CmdQuestion*>(cmd);
            if (questionCmd->questionType() == CmdQuestion::CYCLE)
            {
                int TODO; // depending on role and shape type
            }
            else // IF-QUESTION
            {
                Command* down = cmd->nextCommand(ValencyPoint::Down);
                Command* right = cmd->nextCommand(ValencyPoint::Right);
                Command* underArrow = cmd->nextCommand(ValencyPoint::UnderArrow);

                QList<Command*> stopList = stopDrawingCommands;

                if (underArrow)
                {
                    stopList.push_back(underArrow);
                }

                QRect downRect;
                QRect rightRect;
                QRect underArrowRect;

                if (down && down != underArrow)
                {
                    QPoint cell = item->cell();
                    cell.setY(cell.y() + 1);

                    ShapeItem* shape = addShape(down, cell, item);
                    item->setChildShape(shape, ValencyPoint::Down);

                    if (shape->command()->type() != DRAKON::GO_TO_BRANCH)
                    {
                        drawChildren(shape, stopList, drawGoToBranch && underArrow == Q_NULLPTR);
                    }

                    downRect = shape->rect();
                }

                if (right && right != underArrow)
                {
                    QPoint cell = item->cell();
                    cell.setY(cell.y() + 1);

                    if (down)
                    {
                        cell.setX(cell.x() + downRect.width());
                    }
                    else
                    {
                        cell.setX(cell.x() + 1);
                    }

                    ShapeItem* shape = addShape(right, cell, item);
                    item->setChildShape(shape, ValencyPoint::Right);

                    if (shape->command()->type() != DRAKON::GO_TO_BRANCH)
                    {
                        drawChildren(shape, stopList, drawGoToBranch && underArrow == Q_NULLPTR);
                    }

                    rightRect = shape->rect();
                }

                if (underArrow)
                {
                    int maxHeight = qMax(downRect.height(), rightRect.height());
                    QPoint cell = item->cell();
                    cell.setY(cell.y() + maxHeight + 1);

                    ShapeItem* shape = addShape(underArrow, cell, item);
                    item->setChildShape(shape, ValencyPoint::UnderArrow);

                    if (shape->command()->type() != DRAKON::GO_TO_BRANCH)
                    {
                        drawChildren(shape, stopDrawingCommands, drawGoToBranch);
                    }

                    underArrowRect = shape->rect();
                }

                QRect newRect = downRect.united(rightRect);
                newRect = newRect.united(underArrowRect);
                newRect.setTop(newRect.top() - 1);
                item->setRect(newRect, false);
            }
        }
    }
}

ShapeItem* CyclogramWidget::addCommand(DRAKON::IconType type, const ValencyPoint& point, int param /*= -1*/)
{
    ValencyPoint::Role role = point.role();
    if (type == DRAKON::BRANCH_BEGIN && role == ValencyPoint::Right)
    {
        return addNewBranch(point.owner());
    }

    // 1. Create new command
    Command* newCmd = mCurrentCyclogram.lock()->createCommand(type, param);
    if (!newCmd)
    {
        return Q_NULLPTR;
    }

    ShapeItem* owner = point.owner();
    Command* pointCmd = owner->command();

    // 2. Update command tree connections in logic
    if (pointCmd->type() == DRAKON::BRANCH_BEGIN && newCmd->type() == DRAKON::GO_TO_BRANCH) // new branch creation
    {
        pointCmd->replaceCommand(newCmd, role);
    }
    else
    {
        newCmd->setRole(role);
        pointCmd->insertCommand(newCmd, role);
    }

    // 3. Create new shape and add it to diagram
    ShapeItem* prevChildShape = owner->childShape(role);
    QPoint newCmdCell;

    //3.1 calculate new shape position
    if (prevChildShape) // if valency point already has shape connected
    {
        newCmdCell.setX(prevChildShape->rect().left());
        newCmdCell.setY(prevChildShape->rect().top());
    }
    else // question branch end command
    {
        if (pointCmd->type() == DRAKON::QUESTION)
        {
            CmdQuestion* questionCmd = qobject_cast<CmdQuestion*>(pointCmd);
            if (questionCmd->questionType() == CmdQuestion::CYCLE)
            {
                int TODO; // depending on role and shape type
            }
            else // IF-QUESTION
            {
                newCmdCell = owner->cell();
                newCmdCell.setY(newCmdCell.y() + 1);

                if (role == ValencyPoint::Right)
                {
                    ShapeItem* down = owner->childShape(ValencyPoint::Down);
                    if (down)
                    {
                        newCmdCell.setX(newCmdCell.x() + down->rect().width());
                    }
                    else
                    {
                        newCmdCell.setX(newCmdCell.x() + 1);
                    }
                }
                // UnderArrow always has child shape, or there is no UnderArrow valency point
            }
        }
        else
        {
            if (owner->rect().height() > 1) // if owner is expanded and hadn't child shapes, create new shape in owners cell
            {
                newCmdCell = owner->cell();
            }
            else // create new shape below the points' owner, if it is not expanded
            {
                newCmdCell = owner->cell();
                newCmdCell.setY(newCmdCell.y() + 1);
            }
        }
    }

    // 3.2 create shape
    ShapeItem* newShape = addShape(newCmd, newCmdCell, owner);

    // 3.3 update all rects (child and parent recursively)
    if (prevChildShape)
    {
        // 3.3.1 update shape connections
        if (newShape->command()->type() == DRAKON::QUESTION)
        {
            CmdQuestion* command = qobject_cast<CmdQuestion*>(newShape->command());
            if (command->questionType() == CmdQuestion::CYCLE || command->questionType() == CmdQuestion::SWITCH_STATE)
            {
                newShape->setChildShape(prevChildShape, ValencyPoint::Down);
            }
            else
            {
                newShape->setChildShape(prevChildShape, ValencyPoint::UnderArrow);
            }
        }
        else
        {
            newShape->setChildShape(prevChildShape, ValencyPoint::Down);
        }

        owner->setChildShape(newShape, role);

        // 3.3.2 update child rects, then parent rects
        prevChildShape->pushDown();

        QRect rect = prevChildShape->rect();
        rect.setTop(rect.top() - 1);
        newShape->setRect(rect, false);
    }
    else // if owner hadn't any child shapes, update only to parent direction (QUESTION-IF branches ending)
    {
        // simple command in the end of the one of the QUESTION branches
        // if owner is expanded and hadn't child shapes before, push the expansion to new shape
        if (pointCmd->type() != DRAKON::QUESTION && owner->rect().height() > 1)
        {
            // update owner cell
            QPoint ownerCell = owner->cell();
            ownerCell.setY(owner->rect().top());
            owner->setCell(ownerCell);

            // update new child shape rect
            QRect newShapeRect = owner->rect();
            newShapeRect.setTop(newShapeRect.top() + 1);
            newShape->setRect(newShapeRect, false);
        }

        owner->setChildShape(newShape, role);
    }

    owner->onChildRectChanged(newShape);

    if (type == DRAKON::QUESTION && param == CmdQuestion::SWITCH_STATE)
    {
        ShapeItem* goToBranchItem = addCommand(DRAKON::GO_TO_BRANCH, newShape->valencyPoint(ValencyPoint::Right));
        //goToBranchItem->command()->replaceCommand(newBranchItem->command());
    }

    update();

    return newShape;
}

ShapeItem* CyclogramWidget::addNewBranch(ShapeItem* item)
{
    // Create and add BRANCH_BEGIN item
    auto cyclogram = mCurrentCyclogram.lock();
    Command* newCmd = cyclogram->createCommand(DRAKON::BRANCH_BEGIN);
    if (!newCmd)
    {
        return Q_NULLPTR;
    }

    // save branches order by moving the newly added branch start after the branch-owner of the valency point
    cyclogram->moveLastCommand(item->command());

    //generate unique branch name
    CmdStateStart* cmd = qobject_cast<CmdStateStart*>(newCmd);
    cmd->setText(generateBranchName());

    // create new branch to the right of the item command tree
    QPoint newCmdCell = item->cell();
    newCmdCell.setX(newCmdCell.x() + item->rect().width());

    ShapeItem* newBranchItem = addShape(newCmd, newCmdCell, mRootShape);
    mRootShape->addChildShape(newBranchItem);

    // Create and add GO_TO_BRANCH item to the created branch (by default new branch is linked to itself)
    ShapeItem* goToBranchItem = addCommand(DRAKON::GO_TO_BRANCH, newBranchItem->valencyPoint(ValencyPoint::Down));
    goToBranchItem->command()->replaceCommand(newBranchItem->command());

    onNeedUpdate();

    return newBranchItem;
}

void CyclogramWidget::deleteCommand(ShapeItem* item)
{
    if (item->command()->type() == DRAKON::BRANCH_BEGIN)
    {
        deleteBranch(item);
        return;
    }

    item->remove();
    mCurrentCyclogram.lock()->deleteCommand(item->command()); // shape will be deleted by the signal
}

void CyclogramWidget::deleteBranch(ShapeItem* item)
{
    // 1. Remove cyclogram parent-child link to branch being deleted
    item->parentShape()->removeChildShape(item);

    // 2. Update links of all GO_TO_BRANCH commands
    QRect rect = item->rect();
    int min = rect.left();
    int max = rect.right() + 1;

    foreach (ShapeItem* it, mCommands)
    {
        Command* cmd = it->command();
        if (cmd->type() == DRAKON::GO_TO_BRANCH)
        {
            int x = it->cell().x();
            if (x >= max || x < min) // other branches commands
            {
                if (cmd->nextCommand() == item->command())
                {
                    cmd->replaceCommand(Q_NULLPTR); // remove link to branch being deleted
                }
            }
            else // deleting branch commands
            {
                cmd->replaceCommand(Q_NULLPTR); // remove links to other branches
            }
        }
    }

    // 3. Shift left all branches to the right of the being deleted by the rect of the deleted
    int xOffset = min - max;
    foreach (ShapeItem* it, mCommands)
    {
        if (it->cell().x() >= max)
        {
            QPoint cell = it->cell();
            cell.setX(cell.x() + xOffset);
            it->setCell(cell);

            QRect rect = it->rect();
            rect.setLeft(rect.left() + xOffset);
            rect.setRight(rect.right() + xOffset);
            it->setRect(rect, false);
        }
    }

    // 4. Kill all shapes and commands, belonging to deleting branch
    mCurrentCyclogram.lock()->deleteCommand(item->command(), true);

    QRect r = mRootShape->rect();
    r.setRight(r.right() + xOffset);
    mRootShape->setRect(r, false);

    // 5. Update entire cyclogram rect
    mRootShape->adjust();
}

void CyclogramWidget::setDialogParent(QWidget* widget)
{
    int TODO; // what in case of replace parent?
    mDialogParent = widget;
}

void CyclogramWidget::setParentTitle(const QString& title)
{
    mParentTitle = title;
}
