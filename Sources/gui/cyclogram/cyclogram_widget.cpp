#include <QtWidgets>
#include <QDebug>
#include <QFile>

#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/gui/cyclogram/dialogs/text_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/command_error_dialog.h"
#include "Headers/app_settings.h"

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
#include "Headers/gui/tools/cyclogram_chart_dialog.h"
#include "Headers/gui/editor_window.h"

#include "Headers/gui/cyclogram/shape_item.h"

#include "Headers/logger/Logger.h"

#include "Headers/file_reader.h"

namespace
{
}

CyclogramWidget::CyclogramWidget(QWidget* parent):
    QWidget(parent),
    mSihlouetteLine(Q_NULLPTR),
    mSihlouetteArrow(Q_NULLPTR),
    mCurSubprogram(Q_NULLPTR),
    mMainWindow(Q_NULLPTR),
    mParentScrollArea(Q_NULLPTR),
    mSelectedShape(Q_NULLPTR),
    mDraggingShape(Q_NULLPTR),
    mPressedShape(Q_NULLPTR),
    mItemToCopy(Q_NULLPTR),
    mMouseButtonState(Qt::NoButton),
    mCurrentCommandType(-1)
{
    onAppSettingsChanged();

    mScale = AppSettings::instance().settingValue(AppSettings::CYCLOGRAM_WIDGET_SCALE_DEFAULT).toDouble();

    connect(&AppSettings::instance(), SIGNAL(settingsChanged()), this, SLOT(onAppSettingsChanged()));

    setMouseTracking(true);
    setBackgroundRole(QPalette::Base);

    QPalette pal(QGuiApplication::palette());
    pal.setColor(QPalette::Base, QColor::fromRgba(0xffdfdfdf));
    setPalette(pal);

    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    setSelectedItem(0);
    mRootShape = 0;

    setFocusPolicy(Qt::ClickFocus);
    setAcceptDrops(true);
}

CyclogramWidget::~CyclogramWidget()
{
    clear(true);
}

void CyclogramWidget::clear(bool onDestroy)
{
    if (onDestroy)
    {
        setSelectedItem(0);
    }

    clearSelection();

    qDeleteAll(mCommands);
    mCommands.clear();

    if (!onDestroy && mCyclogram.data())
    {
        disconnect(mCyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
        disconnect(mCyclogram.data(), SIGNAL(deleted(Command*)), this, SLOT(removeShape(Command*)));
    }

    mCyclogram.clear();
}

bool CyclogramWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

        ValencyPoint point;
        int index = commandAt(helpEvent->pos());
        if (index != -1)
        {
            QToolTip::showText(helpEvent->globalPos(), mCommands[index]->toolTip());
        }
        else if (hasValencyPointAt(helpEvent->pos(), point))
        {
            QToolTip::showText(helpEvent->globalPos(), tr("Click to add command"));
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

void CyclogramWidget::resizeEvent(QResizeEvent * /* event */)
{
}

void CyclogramWidget::wheelEvent(QWheelEvent *event)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
    {
        event->accept();

        updateScale(event->pos(), event->delta() / QWheelEvent::DefaultDeltasPerStep);
    }

//    if (QApplication::keyboardModifiers() & Qt::AltModifier)
//    {
//        if (mParentScrollArea)
//        {
//            int vMin = mParentScrollArea->verticalScrollBar()->minimum();
//            int vCur = mParentScrollArea->verticalScrollBar()->value();
//            int vMax = mParentScrollArea->verticalScrollBar()->maximum();
//            int hMin = mParentScrollArea->horizontalScrollBar()->minimum();
//            int hCur = mParentScrollArea->horizontalScrollBar()->value();
//            int hMax = mParentScrollArea->horizontalScrollBar()->maximum();
//            QSize widgetSize = this->size();
//            QSize viewportSize = mParentScrollArea->viewport()->size();
//            LOG_INFO(QString("Scale=%1").arg(mScale));
//            LOG_INFO(QString("Cursor pos (viewport)(x=%1, y=%2)").arg(event->pos().x()).arg(event->pos().y()));
//            LOG_INFO(QString("Cursor pos (widget)  (x=%1, y=%2)").arg(event->pos().x() / mScale).arg(event->pos().y() / mScale));
//            LOG_INFO(QString("Vertical (min=%1, cur=%2, max=%3)").arg(vMin).arg(vCur).arg(vMax));
//            LOG_INFO(QString("Horizontal (min=%1, cur=%2, max=%3)").arg(hMin).arg(hCur).arg(hMax));
//            LOG_INFO(QString("Widget size (w=%1, h=%2)").arg(widgetSize.width()).arg(widgetSize.height()));
//            LOG_INFO(QString("Viewport size (w=%1, h=%2)").arg(viewportSize.width()).arg(viewportSize.height()));
//            LOG_INFO(QString("============================="));
//            event->accept();
//        }
//    }
}

void CyclogramWidget::updateScale(const QPoint& cursorPos, int numSteps)
{
    if (!mParentScrollArea)
    {
        return;
    }

    qreal scaleBefore = mScale;
    qreal scaleChangeStep = AppSettings::instance().settingValue(AppSettings::CYCLOGRAM_WIDGET_SCALE_STEP).toDouble();
    qreal maxScale = AppSettings::instance().settingValue(AppSettings::CYCLOGRAM_WIDGET_SCALE_MAX).toDouble();
    qreal minScale = AppSettings::instance().settingValue(AppSettings::CYCLOGRAM_WIDGET_SCALE_MIN).toDouble();

    mScale += numSteps * scaleChangeStep;

    if (mScale > maxScale)
    {
        mScale = maxScale;
    }

    if (mScale < minScale)
    {
        mScale = minScale;
    }

    // save necessary data before scaling
    QScrollBar* vBar = mParentScrollArea->verticalScrollBar();
    QScrollBar* hBar = mParentScrollArea->horizontalScrollBar();
    int vValueBefore = vBar->value();
    int hValueBefore = hBar->value();
    QPoint viewportMin(hValueBefore, vValueBefore);
    QPoint viewportPos = cursorPos - viewportMin; // cursor position in viewport coordinates, it must be constant (if possible) to be "focus point"

    // scale widget
    onNeedUpdate();

    // set horizontal and vertical scrollbar values after scaling, still focusing (if possible) on current cursor position
    QPoint newCursorPos = cursorPos * mScale / scaleBefore;
    QPoint newViewportMin = newCursorPos - viewportPos;

    int newHValue = newViewportMin.x();
    int newVValue = newViewportMin.y();

    // check new viewport borders
    if (newHValue < 0)
    {
        newHValue = 0;
    }

    if (newHValue > hBar->maximum())
    {
        newHValue = hBar->maximum();
    }

    if (newVValue < 0)
    {
        newVValue = 0;
    }

    if (newVValue  > vBar->maximum())
    {
        newVValue = vBar->maximum();
    }

    if (hBar->value() != newHValue)
    {
        hBar->setValue(newHValue);
    }

    if (vBar->value() != newVValue)
    {
        vBar->setValue(newVValue);
    }
}

void CyclogramWidget::deleteSelectedItem()
{
    if (!mSelectedShape)
    {
        return;
    }

    QString errorDesc;
    if (canBeDeleted(mSelectedShape, errorDesc))
    {
        bool isBranch = mSelectedShape->command()->type() == DRAKON::BRANCH_BEGIN;
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
            ShapeItem* item = mSelectedShape;
            clearSelection();
            deleteCommand(item);
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Command deletion"), tr("Command can not be deleted!\nReason: ") + errorDesc);
    }
}

void CyclogramWidget::keyPressEvent(QKeyEvent *event)
{
    //TODO данный обработчик вызывается только при получении фокуса виджетом (на него нужно кликнуть, проверить)
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
            if (mSelectedShape)
            {
                if ((mSelectedShape->command()->flags() & Command::Editable) != 0)
                {
                    showEditDialog(mSelectedShape->command());
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

//    if (mMovingItem)
//    {
//        QList<ShapeItem*> moving;
//        moving.push_back(mMovingItem);
//        drawItems(moving, painter);
//    }
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
    mMouseButtonState = event->button();
    mDragStartPosition = event->pos();

    // no interaction while cyclogram running
    if (mCyclogram.lock()->state() == Cyclogram::RUNNING)
    {
        return;
    }

    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
    {
        int index = commandAt(event->pos());
        if (index >= 0)
        {
            mPressedShape = mCommands[index];
            mPreviousPosition = event->pos();
        }
    }
}

void CyclogramWidget::onClickVP(const ValencyPoint& point, const QPoint& pos)
{
    clearSelection();

    ShapeItem* item = point.owner();
    DRAKON::IconType type = item->command()->type();

    if (type == DRAKON::BRANCH_BEGIN && point.role() == ValencyPoint::Right)
    {
        addNewCommand(type, point); // just add new branch immediately
    }
    else // add some command via dialog (TODO tempotary)
    {
        if (mCurrentCommandType != -1)
        {
            addNewCommand(DRAKON::IconType(mCurrentCommandType), point, CmdQuestion::IF); //TODO not SWITCH_STATE
        }
        else // TODO show context menu?
        {
            showContextMenuForVP(point, pos);
        }

        //TODO else?

//        ShapeAddDialog dialog(this);

//        dialog.setValencyPoint(point);
//        dialog.exec();

//        if (dialog.result() == QDialog::Accepted)
//        {
//            addNewCommand(dialog.shapeType(), point, dialog.param());
//        }
    }
}

void CyclogramWidget::showContextMenuForVP(const ValencyPoint& point, const QPoint& pos)
{
    clearSelection();

    ShapeItem* item = point.owner();
    DRAKON::IconType type = item->command()->type();
    if (type == DRAKON::BRANCH_BEGIN && point.role() == ValencyPoint::Right)
    {
        QMenu menu(this);
        QAction* addNewBranchAction = menu.addAction(tr("Add BRANCH"));
        addNewBranchAction->setData(int(DRAKON::BRANCH_BEGIN));

        menu.addSeparator();
        QAction* pasteAction = menu.addAction(tr("Paste"));
        pasteAction->setEnabled(mItemToCopy != Q_NULLPTR);

        QAction* action = menu.exec(pos);
        if (!action)
        {
            return;
        }

        if (action == addNewBranchAction)
        {
            addNewCommand(DRAKON::BRANCH_BEGIN, point);
        }

        if (action == pasteAction)
        {
            if (mItemToCopy->command()->type() == DRAKON::BRANCH_BEGIN)
            {
                copyBranchTo(mItemToCopy, point);
            }
            else
            {
                QMessageBox::warning(this, tr("Error"), tr("Selected command can not be copied here"));
            }
            //copyCommandTo(mItemToCopy, point);
        }

        return;
    }

    QMenu menu(this);
    menu.addAction(tr("Add MODULE COMMAND"))->setData(int(DRAKON::ACTION_MODULE));
    menu.addAction(tr("Add MATH COMMAND"))->setData(int(DRAKON::ACTION_MATH));
    menu.addAction(tr("Add SUBPROGRAM"))->setData(int(DRAKON::SUBPROGRAM));
    QAction* questionAct = menu.addAction(tr("Add QUESTION"));
    questionAct->setData(int(DRAKON::QUESTION));
    menu.addAction(tr("Add DELAY"))->setData(int(DRAKON::DELAY));
    menu.addAction(tr("Add OUTPUT"))->setData(int(DRAKON::OUTPUT));

    if (point.canBeLanded())
    {
        menu.addAction(tr("Add SWITCH STATE"))->setData(int(DRAKON::QUESTION)); //TODO remove and add "Add GO_TO_BRANCH"
        //TODO cycle?
    }

    menu.addSeparator();
    QAction* pasteAction = menu.addAction(tr("Paste"));

    pasteAction->setEnabled(mItemToCopy != Q_NULLPTR);

    QAction* action = menu.exec(pos);
    if (!action)
    {
        return;
    }

    if (action == pasteAction)
    {
        copyCommandTo(mItemToCopy, point);
        return;
    }

    int param = -1;
    int command = action->data().toInt();
    if (command == DRAKON::QUESTION) //TODO remove
    {
        if (action == questionAct)
        {
            param = CmdQuestion::IF;
        }
        else
        {
            param = CmdQuestion::SWITCH_STATE;
        }

//      mParam = CmdQuestion::CYCLE; //TODO
    }

    addNewCommand(DRAKON::IconType(command), point, param);
}

void CyclogramWidget::copyCommandTo(ShapeItem* itemToCopy, const ValencyPoint& point)
{
    Command* commandToCopy = itemToCopy->command();
    DRAKON::IconType typeToCopy = commandToCopy->type();
    bool isBranchRightVP = (point.owner()->command()->type() == DRAKON::BRANCH_BEGIN && point.role() == ValencyPoint::Right);

    if (isBranchRightVP)
    {
        if (typeToCopy == DRAKON::BRANCH_BEGIN && !Cyclogram::isCyclogramEndBranch(itemToCopy->command()))
        {
            copyBranchTo(itemToCopy, point);
        }
        else
        {
            QMessageBox::warning(this, tr("Error"), tr("Selected item can not be copied here"));
        }

        return;
    }
    else
    {
        if (typeToCopy == DRAKON::BRANCH_BEGIN)
        {
            QMessageBox::warning(this, tr("Error"), tr("Selected item can not be copied here"));
            return;
        }
    }

    bool canBePasted = true;

    if (typeToCopy == DRAKON::GO_TO_BRANCH)
    {
        point.canBeLanded();
        int TODO; // can be pasted only to points that can be landed
        canBePasted = false;
    }

    if (!canBePasted)
    {
        QMessageBox::warning(this, tr("Error"), tr("Selected item can not be copied here"));
        return;
    }

    //DRAKON::TERMINATOR - this type of command can not be copied
    //DRAKON::BRANCH_BEGIN - this type of command is copied as entire branch

    // create command and load it from another
    Command* newCmd = mCyclogram.lock()->createCommand(typeToCopy, CmdQuestion::IF);
    newCmd->copyFrom(commandToCopy);

    if (!newCmd)
    {
        LOG_ERROR(QString("Command copy not created"));
        return;
    }

    if (typeToCopy == DRAKON::QUESTION) // TODO temporary hack for QUESTION-SWITCH_STATE copying
    {
        CmdQuestion* from = qobject_cast<CmdQuestion*>(commandToCopy);
        CmdQuestion* to = qobject_cast<CmdQuestion*>(newCmd);

        if (from->questionType() == CmdQuestion::SWITCH_STATE) // "switch state" -> "if"
        {
            to->setQuestionType(CmdQuestion::IF);
        }
    }

    ShapeItem* newShape = addCommand(newCmd, point);
    update();
}

void CyclogramWidget::copyBranchTo(ShapeItem* itemToCopy, const ValencyPoint& point)
{
    auto cyclogram = mCyclogram.lock();
    Command* newBranchCmd = cyclogram->createBranchCopy(itemToCopy->command(), point);

    if (!newBranchCmd)
    {
        LOG_ERROR(QString("Branch copy not created"));
        return;
    }

    // save branches order by moving the newly added branch start after the branch-owner of the valency point
    cyclogram->changeCommandsOrder(newBranchCmd, point.owner()->command());

    QRect ownerRect = point.owner()->rect();
    QPoint cell(point.owner()->cell().x() + ownerRect.width(), 1);

    ShapeItem* newBranchCopy = copyBranch(newBranchCmd, cell);
    mRootShape->addChildShape(newBranchCopy);

    int maxHeight = mRootShape->rect().bottom();

    QRect newRect = newBranchCopy->rect();
    if (newRect.height() > maxHeight)
    {
        maxHeight = newRect.height();
    }

    auto branchesShapes = mRootShape->childShapes();

    // shift branches to the right of the inserted
    foreach (ShapeItem* it, branchesShapes)
    {
        if (it == newBranchCopy)
        {
            continue;
        }

        QPoint cell = it->cell();
        QPoint newBranchCell = newBranchCopy->cell();
        if (cell.x() < newBranchCell.x())
        {
            continue;
        }

        QRect rect = it->rect();
        rect.setRight(rect.right() + newBranchCopy->rect().width());
        rect.setLeft(rect.left() + newBranchCopy->rect().width());
        it->setRect(rect, true);
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
    }

    // set root shape rect
    QRect rect = mRootShape->rect();
    rect.setRight(rect.right() + itemToCopy->rect().width());
    rect.setBottom(maxHeight);
    mRootShape->setRect(rect, false);

    onNeedUpdate();
}

void CyclogramWidget::moveBranchTo(ShapeItem* branchToMove, const ValencyPoint& point)
{
    bool canBeMoved = (point.owner()->command()->type() == DRAKON::BRANCH_BEGIN && point.role() == ValencyPoint::Right);
    if (!canBeMoved)
    {
        QMessageBox::warning(this, tr("Error"), tr("Selected branch can not be moved here"));
        return;
    }

    bool isEndBranch = Cyclogram::isCyclogramEndBranch(branchToMove->command());
    if (isEndBranch)
    {
        QMessageBox::warning(this, tr("Error"), tr("End branch can not be moved anywhere"));
        return;
    }

    // save branches order by moving the newly added branch start after the branch-owner of the valency point
    auto cyclogram = mCyclogram.lock();
    cyclogram->changeCommandsOrder(branchToMove->command(), point.owner()->command());

    QRect ownerRect = point.owner()->rect();
    QPoint cell(point.owner()->cell().x() + ownerRect.width(), 1);

    int oldCell = branchToMove->cell().x();
    int xOffset = cell.x() - oldCell;

    if (xOffset == 0)
    {
        return;
    }

    if (xOffset > 0)
    {
        xOffset -= branchToMove->rect().width();
    }

    QRect movingBranchRect = branchToMove->rect();
    movingBranchRect.setLeft(movingBranchRect.left() + xOffset);
    movingBranchRect.setRight(movingBranchRect.right() + xOffset);
    branchToMove->setRect(movingBranchRect, true);

    int otherBranchesOffset = 0;
    if (xOffset > 0)
    {
       otherBranchesOffset -= branchToMove->rect().width();
    }
    else
    {
       otherBranchesOffset += branchToMove->rect().width();
    }

    auto branchesShapes = mRootShape->childShapes();

    foreach (ShapeItem* it, branchesShapes)
    {
        if (it == branchToMove) // skip moving branch (it is already moved)
        {
            continue;
        }

        QPoint cell = it->cell();
        QPoint newBranchCell = branchToMove->cell();

        if (xOffset > 0)
        {
            // do not move branches to the right of the insertion point
            // if they do not intersect moving branch rect
            if (cell.x() > newBranchCell.x() && cell.x() > branchToMove->rect().right())
            {
                continue;
            }

            if (cell.x() < oldCell) // do not move branches to the left of the old position
            {
                continue;
            }
        }
        else
        {
            if (cell.x() < newBranchCell.x()) // do not move branches to the left of the insertion point
            {
                continue;
            }

            if (cell.x() > oldCell) // do not move branches to the right of the old position
            {
                continue;
            }
        }

        QRect rect = it->rect();
        rect.setRight(rect.right() + otherBranchesOffset);
        rect.setLeft(rect.left() + otherBranchesOffset);
        it->setRect(rect, true);

        if (it->cell().x() == 0) // new "start" branch changed, update command hierarchy
        {
            Command* first = cyclogram->first();
            first->replaceCommand(it->command());
        }
    }

    onNeedUpdate();
}

ShapeItem* CyclogramWidget::copyBranch(Command* branchCmd, const QPoint& cell)
{
    ShapeItem* shape = addShape(branchCmd, cell, mRootShape);
    QList<Command*> stopList;
    drawChildren(shape, stopList, true);

    return shape;
}

void CyclogramWidget::showContextMenuForCommand(ShapeItem* item, const QPoint& pos)
{
    DRAKON::IconType type = item->command()->type();

    QMenu menu(this);

    QString editText = tr("Edit");
    QString deleteText = tr("Delete");
    QString copyText = (type == DRAKON::BRANCH_BEGIN) ? tr("Copy entire branch") : tr("Copy");

    menu.addAction(editText);
    menu.addAction(deleteText);
    menu.addAction(copyText);

    bool enableCopyAction = true;
    if (type == DRAKON::TERMINATOR || type == DRAKON::QUESTION) //TODO temporary disable question command copy
    {
        enableCopyAction = false; // no copy action available, cyclogram can have only one start and one end
    }

    if (type == DRAKON::BRANCH_BEGIN)
    {
        auto cyclogram = mCyclogram.lock();
        enableCopyAction = !cyclogram->isCyclogramEndBranch(item->command());
    }

    QAction* action = menu.exec(pos);
    if (!action)
    {
        return;
    }

    if (action->text() == editText)
    {
        showEditDialog(item->command());
        return;
    }

    if (action->text() == deleteText)
    {
        deleteSelectedItem();
        return;
    }

    if (action->text() == copyText)
    {
        if (!enableCopyAction)
        {
            QMessageBox::warning(this, tr("Error"), tr("Selected item can not be copied"));
            return;
        }

        mItemToCopy = item;
        return;
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

    Q_ASSERT(mMainWindow);

    EditorWindow* mainWindow = qobject_cast<EditorWindow*>(mMainWindow);

    SubProgramDialog* subProgramDialog = mainWindow->subprogramDialog(mCurSubprogram);
    if (subProgramDialog)
    {
        subProgramDialog->activateWindow();
        subProgramDialog->raise();
    }
    else
    {
        subProgramDialog = new SubProgramDialog(mCurSubprogram, mCyclogram.lock(), mMainWindow);
        mainWindow->addSuprogramDialog(mCurSubprogram, subProgramDialog);

        subProgramDialog->cyclogramWidget()->setCurrentCommandType(mCurrentCommandType);

        QString title = updateWindowTitle(subProgramDialog);
        subProgramDialog->cyclogramWidget()->setWindowTitle(title);
        subProgramDialog->show();

        connect(mCurSubprogram, SIGNAL(dataChanged(const QString&)), subProgramDialog, SLOT(onCommandTextChanged(const QString&)));
        connect(this, SIGNAL(windowTitleChanged(const QString&)), subProgramDialog, SLOT(onParentWindowTitleChanged(const QString&)));
    }
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

    title = windowTitle();

    if (!title.isEmpty())
    {
        title += CyclogramWidget::delimiter();
    }

    title += mCurSubprogram->text();
    title += "[*]";
    dialog->setWindowTitle(title);

    return title;
}

void CyclogramWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    //remember that mousePressEvent will be called first!
    if (event->button() == Qt::LeftButton)
    {
        if (mCyclogram.lock()->state() == Cyclogram::RUNNING)
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
    bool isRightBtnPressed = ((event->buttons() & Qt::RightButton) > 0);
    bool isLeftBtnPressed = ((event->buttons() & Qt::LeftButton) > 0);

    if (!isRightBtnPressed && !isLeftBtnPressed)
    {
        return;
    }

    if ((event->pos() - mDragStartPosition).manhattanLength() < QApplication::startDragDistance())
    {
        return;
    }

    if (mPressedShape && !mDraggingShape)
    {
        auto cyclogram = mCyclogram.lock();
        DRAKON::IconType type = mPressedShape->command()->type();
        Command* commandCopy = cyclogram->createCommand(type);
        commandCopy->copyFrom(mPressedShape->command());

        mDraggingShape = addShape(commandCopy, mPressedShape->cell(), 0/*mRootShape*/);
    }

    if (mDraggingShape)
    {
        moveItemTo(event->pos());
    }
}

void CyclogramWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (mCyclogram.lock()->state() == Cyclogram::RUNNING)
    {
        return; // no cyclogram mouse interaction available during cyclogram running
    }

    bool isRightBtnPressed = ((mMouseButtonState & Qt::RightButton) > 0);
    bool isLeftBtnPressed = ((mMouseButtonState & Qt::LeftButton) > 0);

    // move dragging item to valency point with left mouse button
    if (isLeftBtnPressed)
    {
        ValencyPoint point;
        bool isOverVP = hasValencyPointAt(event->pos(), point);

        if (mDraggingShape) // drag-and-drop with LMB
        {
            if (isOverVP)
            {
                // try to move dragging shape to valency point above
                if (mPressedShape->command()->type() == DRAKON::BRANCH_BEGIN)
                {
                    moveBranchTo(mPressedShape, point);
                }
                else
                {
                    if (canBeMoved(mPressedShape, point))
                    {
                        copyCommandTo(mPressedShape, point);
                        deleteCommand(mPressedShape);
                    }
                    else
                    {
                        QMessageBox::warning(this, tr("Error"), tr("Selected command can not be moved here"));
                    }
                }
            }
        }
        else // just usual click
        {
            if (isOverVP)
            {
                onClickVP(point, mapToGlobal(event->pos()));
            }
            else // if no valency point click, check command shape click
            {
                int index = commandAt(event->pos());
                if (index >= 0)
                {
                    ShapeItem* clickedItem = mCommands[index];
                    mPreviousPosition = event->pos();

                    if (!mSelectedShape || mSelectedShape != clickedItem)
                    {
                        clearSelection(false);
                        setSelectedItem(clickedItem);
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

    // copy dragging item to valency point with right mouse button
    if (isRightBtnPressed)
    {
        ValencyPoint point;
        bool isOverVP = hasValencyPointAt(event->pos(), point);

        if (mDraggingShape) // drag-and-drop with RMB,  try to copy dragging shape to valency point above
        {
            if (isOverVP)
            {
                copyCommandTo(mPressedShape, point);
            }
        }
        else // just usual click, show context menu for "hotspot" object
        {
            clearSelection();

            if (isOverVP)
            {
                showContextMenuForVP(point, mapToGlobal(event->pos()));
            }
            else // if no valency point click, check command shape click
            {
                int index = commandAt(event->pos());
                if (index < 0)
                {
                    return;
                }

                ShapeItem* clickedItem = mCommands[index];

                clearSelection(false);
                setSelectedItem(clickedItem);
                update();

                showContextMenuForCommand(clickedItem, mapToGlobal(event->pos()));
            }
        }
    }

    mPressedShape = 0;
    if (mDraggingShape)
    {
        deleteCommand(mDraggingShape);
        mDraggingShape = 0;
    }

    mMouseButtonState = Qt::NoButton;
    updateCursor(event->pos());
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
    mDraggingShape->setPosition(mDraggingShape->position() + offset);
    mPreviousPosition = pos;

    updateCursor(pos);
    update();
}

void CyclogramWidget::updateCursor(const QPoint& pos)
{
    Qt::CursorShape curShape = this->cursor().shape();

    if (!mDraggingShape || mMouseButtonState == Qt::NoButton)
    {
        if (curShape != Qt::ArrowCursor)
        {
            setCursor(QCursor(Qt::ArrowCursor));
        }

        return;
    }

    bool isRightBtnPressed = ((mMouseButtonState & Qt::RightButton) > 0);
    bool isLeftBtnPressed = ((mMouseButtonState & Qt::LeftButton) > 0);

    if (!isRightBtnPressed && !isLeftBtnPressed)
    {
        return;
    }

    ValencyPoint point;
    if (hasValencyPointAt(pos, point))
    {
        if (curShape == Qt::ArrowCursor)
        {
            QCursor cursor(isLeftBtnPressed ? Qt::DragMoveCursor : Qt::DragCopyCursor);
            setCursor(cursor);
        }
    }
    else
    {
        if (curShape == Qt::DragMoveCursor || curShape == Qt::DragCopyCursor)
        {
            QCursor cursor(Qt::ArrowCursor);
            setCursor(cursor);
        }
    }
}

ShapeItem* CyclogramWidget::addShape(Command* cmd, const QPoint& cell, ShapeItem* parentShape)
{
    ShapeItem* shapeItem = new ShapeItem(this);
    shapeItem->setCommand(cmd);
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
    mScale = AppSettings::instance().settingValue(AppSettings::CYCLOGRAM_WIDGET_SCALE_DEFAULT).toDouble();

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

    if (mCyclogram.data())
    {
        disconnect(mCyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
        disconnect(mCyclogram.data(), SIGNAL(deleted(Command*)), this, SLOT(removeShape(Command*)));
    }

    mCyclogram = cyclogram;
    connect(cyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
    connect(cyclogram.data(), SIGNAL(deleted(Command*)), this, SLOT(removeShape(Command*)));

    ShapeItem* title = addShape(first, QPoint(0, 0), 0);
    mRootShape = title;
    drawCyclogram(title);

    mRootShape->adjust();

    onNeedUpdate();

    //mRealWidgetSize = this->size();
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
    if (mCyclogram.lock()->state() == Cyclogram::RUNNING)
    {
        error = tr("THIS TEXT SHOULD NEVER APPEAR ON THE SCREEN!");
        return false;
    }

    if (item->command()->type() == DRAKON::BRANCH_BEGIN)
    {
        // START and END branches never can be deleted
        Command* startBranch = mCyclogram.lock()->first();

        // check is start branch trying to delete
        if (startBranch->nextCommand() == item->command())
        {
            error = tr("Start branch never can be deleted");
            return false;
        }

        const ShapeItem* lastBranch = findBranch(mCyclogram.lock()->last());

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

    if (!mSelectedShape)
    {
        return;
    }

    setSelectedItem(0);
    if (needUpdate)
    {
        update();
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
    mCyclogram.lock()->deleteCommand(shape->command());
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
            mCurSubprogram = qobject_cast<CmdSubProgram*>(command);
            showSubprogramWidget();
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
        {
            LOG_WARNING(QString("Default shape edit dualog launched"));
            TextEditDialog* shapeEditDialog = new TextEditDialog(TextEditDialog::SHAPE_EDIT, this);
            shapeEditDialog->setCommand(command);
            dialog = shapeEditDialog;
        }
        break;
    }

    if (dialog)
    {
        dialog->exec();
        dialog->deleteLater();
    }
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
        LOG_ERROR(QString("Not cyclogram start terminator"));
        return;
    }

    QList<Command*> branches;
    mCyclogram.lock()->getBranches(branches);

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

                    if (down && down != underArrow)
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

ShapeItem* CyclogramWidget::addNewCommand(DRAKON::IconType type, const ValencyPoint& point, int param /*= -1*/)
{
    ValencyPoint::Role role = point.role();
    if (type == DRAKON::BRANCH_BEGIN && role == ValencyPoint::Right)
    {
        return addNewBranch(point.owner());
    }

    // 1. Create new command
    Command* newCmd = mCyclogram.lock()->createCommand(type, param);
    if (!newCmd)
    {
        return Q_NULLPTR;
    }

    ShapeItem* newShape = addCommand(newCmd, point);

    if (type == DRAKON::QUESTION && param == CmdQuestion::SWITCH_STATE)
    {
        ShapeItem* goToBranchItem = addNewCommand(DRAKON::GO_TO_BRANCH, newShape->valencyPoint(ValencyPoint::Right));
        //goToBranchItem->command()->replaceCommand(newBranchItem->command());
    }

    update();

    return newShape;
}

ShapeItem* CyclogramWidget::addCommand(Command* cmd, const ValencyPoint &point)
{
    ValencyPoint::Role role = point.role();
    ShapeItem* owner = point.owner();
    Command* pointCmd = owner->command();

    // 2. Update command tree connections in logic
    if (pointCmd->type() == DRAKON::BRANCH_BEGIN && cmd->type() == DRAKON::GO_TO_BRANCH) // new branch creation
    {
        pointCmd->replaceCommand(cmd, role);
    }
    else
    {
        cmd->setRole(role);
        pointCmd->insertCommand(cmd, role);
    }

    // 3. Create new shape and add it to diagram
    QPoint newCmdCell = calculateNewCommandCell(point);
    ShapeItem* newShape = addShape(cmd, newCmdCell, owner);
    updateWidgetShapes(newShape, point);
    return newShape;
}

void CyclogramWidget::updateWidgetShapes(ShapeItem* newShape, const ValencyPoint& point)
{
    ValencyPoint::Role role = point.role();
    ShapeItem* owner = point.owner();
    Command* pointCmd = owner->command();
    ShapeItem* prevChildShape = owner->childShape(role);

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
}

ShapeItem* CyclogramWidget::addNewBranch(ShapeItem* item)
{
    // Create and add BRANCH_BEGIN item
    auto cyclogram = mCyclogram.lock();
    Command* newCmd = cyclogram->createCommand(DRAKON::BRANCH_BEGIN);
    if (!newCmd)
    {
        return Q_NULLPTR;
    }

    // save branches order by moving the newly added branch start after the branch-owner of the valency point
    cyclogram->changeCommandsOrder(newCmd, item->command());

    //generate unique branch name
    CmdStateStart* cmd = qobject_cast<CmdStateStart*>(newCmd);
    cmd->setText(cyclogram->generateBranchName(tr("New Branch")));

    // create new branch to the right of the item command tree
    QPoint newCmdCell = item->cell();
    newCmdCell.setX(newCmdCell.x() + item->rect().width());

    ShapeItem* newBranchItem = addShape(newCmd, newCmdCell, mRootShape);
    mRootShape->addChildShape(newBranchItem);

    // Create and add GO_TO_BRANCH item to the created branch (by default new branch is linked to itself)
    ShapeItem* goToBranchItem = addNewCommand(DRAKON::GO_TO_BRANCH, newBranchItem->valencyPoint(ValencyPoint::Down));
    goToBranchItem->command()->replaceCommand(newBranchItem->command());

    onNeedUpdate();

    return newBranchItem;
}

void CyclogramWidget::deleteCommand(ShapeItem* item)
{
    if (mDraggingShape != item && item->command()->type() == DRAKON::BRANCH_BEGIN)
    {
        deleteBranch(item);
        return;
    }

    item->remove();
    mCyclogram.lock()->deleteCommand(item->command()); // shape will be deleted by the signal
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
    mCyclogram.lock()->deleteCommand(item->command(), true);

    QRect r = mRootShape->rect();
    r.setRight(r.right() + xOffset);
    mRootShape->setRect(r, false);

    // 5. Update entire cyclogram rect
    mRootShape->adjust();
}

void CyclogramWidget::setMainWindow(QWidget* widget)
{
    mMainWindow = widget;
}

void CyclogramWidget::setParentScrollArea(QScrollArea* scroll)
{
    mParentScrollArea = scroll;
}

void CyclogramWidget::onAppSettingsChanged()
{
    ShapeItem::itemSize(true);
    ShapeItem::cellSize(true);
    ShapeItem::origin(true);
}

QString CyclogramWidget::delimiter()
{
    return QString(" -> ");
}

void CyclogramWidget::setSelectedItem(ShapeItem* item)
{
    if (mSelectedShape)
    {
        mSelectedShape->setSelected(false);
    }

    mSelectedShape = item;

    if (mSelectedShape)
    {
        mSelectedShape->setSelected(true);
    }

    emit selectionChanged(mSelectedShape);
}

ShapeItem* CyclogramWidget::selectedItem() const
{
    return mSelectedShape;
}

QPoint CyclogramWidget::calculateNewCommandCell(const ValencyPoint& point)
{
    ValencyPoint::Role role = point.role();
    ShapeItem* owner = point.owner();
    Command* pointCmd = owner->command();
    ShapeItem* prevChildShape = owner->childShape(role);

    QPoint newCmdCell;

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

    return newCmdCell;
}

void CyclogramWidget::dragEnterEvent(QDragEnterEvent* event)
{
    LOG_DEBUG(QString("drag ENTER event"));
}

void CyclogramWidget::dragMoveEvent(QDragMoveEvent* event)
{
    LOG_DEBUG(QString("drag MOVE event"));
}

void CyclogramWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    LOG_DEBUG(QString("drag LEAVE event"));
}

void CyclogramWidget::dropEvent(QDropEvent* event)
{
    LOG_DEBUG(QString("DROP event"));
}

void CyclogramWidget::setCurrentCommandType(int command)
{
    mCurrentCommandType = command;
}

bool CyclogramWidget::canBeMoved(ShapeItem *item, const ValencyPoint &point) const
{
    // only "empty" question command can be moved
    if (item && item->command()->type() == DRAKON::QUESTION)
    {
        CmdQuestion* questionCmd = qobject_cast<CmdQuestion*>(item->command());
        CmdQuestion::QuestionType type = questionCmd->questionType();

        Command* down = questionCmd->nextCommand(ValencyPoint::Down);
        Command* right = questionCmd->nextCommand(ValencyPoint::Right);
        Command* underArrow = questionCmd->nextCommand(ValencyPoint::UnderArrow);

        bool isLanded = (underArrow == Q_NULLPTR);
        bool isEmptyCommand = (down == underArrow && right == underArrow);

        return (isEmptyCommand && !isLanded);
    }

    return true;
}
