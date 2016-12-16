#include <QtWidgets>
#include <QDebug>

#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/gui/cyclogram/dialogs/shape_add_dialog.h"
#include "Headers/gui/cyclogram/dialogs/shape_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/command_error_dialog.h"

#include "Headers/logic/cyclogram.h"
#include "Headers/logic/command.h"
#include "Headers/logic/commands/cmd_title.h"

#include "Headers/logic/commands/cmd_action_math.h"
#include "Headers/logic/commands/cmd_delay.h"
#include "Headers/logic/commands/cmd_state_start.h"
#include "Headers/logic/commands/cmd_set_state.h"
#include "Headers/logic/commands/cmd_question.h"

#include "Headers/gui/cyclogram/dialogs/cmd_action_math_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_delay_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_state_start_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_set_state_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_question_edit_dialog.h"

/* Алгоритм построения ДРАКОН-схемы на основе дерева команд (с нуля)
 *
 * 1. Берем список бранчей
 * 2. Находим первый бранч
 * 3. Создаем для команды BRANCH_BEGIN ShapeItem
 * 4. ShapeItem представляет собой "матрешку" из собственно Shape самой команды и точек валентности (точек, куда можно что-то вставлять)
 * 5. В каждой из точек валентности находится тоже "матрешка"-ShapeItem
 * 6. Если ShapeItem в точке валентности отсутствует, то считаем, что матрешка пустая с нулевым RECTом
 * 7. У каждого шейпа есть функция "отрисуй себя"
 * 8. В зависимости от типа команды шейпа меняется порядок отрисовки
 * 9. Если это не QUESTION, то рисуется шейп самой "матрешки", а затем вызывается отрисовка "матрешки" в точке валентности (DOWN)
 * 10. Если это QUESTION-IF, то сначала отрисовывается шейп, потом отрисовывается DOWN, потом RIGHT, потом они выравниваются по размеру, после этого отрисовывается соединитель и UNDER_ARROW
 * 11. Если DOWN или RIGHT заземлены (матрешка заканчивается GO_TO_BRANCH), то UNDER_ARROW пустой (ветки разъединились)
 * 12. Если это QUESTION-CYCLE, то сначала отрисовывается UNDER_ARROW, потом сам шейп, потом DOWN, потом RIGHT, потом стрелка
 * (стрелку пока неясно к какому шейпу цеплять снизу - зависит от "выходной точки" матрешки - наверное надо считать)
 * 13. Выравнивание по размеру - если ректы DOWN и RIGHT на совпали, то в меньший set'аем размер большего
 * 14. Меньший рекурсивно проталкивает размеры ректа внутрь матрешки, уменьшая на каждом шаге высоту на 1
 * 15. Если в процессе встречается QUESTION, то, если IF - пушим в UNDER_ARROW (если не пустой), в остальных случаях пушим в DOWN и RIGHT (вероятно заземление как-то повлияет)
 * 16. В общем случае все пуши должны повлиять на ректы GO_TOшек
 * 17. Это все касается отрисовки одного бранча
 * 18. Далее рассмотрим как отрисовать все бранчи
 * 19. В процессе отрисовки первого бранча формируем список бранчей слева направо (как они получаются, но ендовый бранч всегда последний)
 * 20. В этом порядке отрисовываем бранчи
 * 21. После того, как первый бранс отрисовался, нам известен его РЕКТ
 * 22. Следующий бранч рисуем правее этого ректа
 * 23. Когда мы отрисовали бранчи из списка, рисуем все бранчи, которые в список не попали (за исключением ендового), а потом ендовый
 * 24. При отрисовке бранчей, находим самый "длинный" и сетим его рект во все остальные бранчи
 * 25. После этого рисуем стрелку силуэта
 *
*/

/* Добавление и удаление команды в новой системе:
 *
 * 1. Добавление в общем-то простое
 * 2. Новый шейп добавляется в "матрешку"
 * 3. Если матрешка "конечная" (не имеет в DOWN шейпов) и высота ее шейпа 1 (не "растянутый" шейп), то в DOWN этой матрешки добавляется новый шейп (на деле посмотреть)
 * 4. Обновляется рект и парентовый шейп рекурсивно пинается
 * 5. В результате обновляется рект бранча
 * 6. Если он увеличился (добавилась команда в нерастянутую ветку), то пинаем setRect'ом остальные бранчи (они подрастянутся)
 *
 * 1. Удаление будет вероятно посложнее
 * 2. Удаляем шейп из матрешки
 * 3. Если чайлдовый шейп растянутый, то мы его просто рект удлиняем еще на 1
 * 4. Если не растянутый, то уменьшаем шейп и пинаем парента
 * 5. Если по пути натыкаемся на QUESTION, то смотрим его тип и бранч откуда мы пришли.
 * 6. Пытаемся засетать бранч, который должен совпадать по размеру
 * 7. Если засетался (бранч был растянутым), то пинаем дальше вверх по иерархии с обновлением бранчей и т.д.
 * 8. Если не засетался, то обратно пихаем сет рект, растягивая бранч, в которм удалили команду
 * 9. Если удаляем QUESTION, то удаляем RIGHT бранч, обновляя размеры нужных матрешек (вверх-вниз по иерерхии, бранчи и т.д.)
 *
 * Если это не взлетит, то можно тупо перерисовывать циклограмму целиком при каждом изменении (первый вариант очистить-загрузить надо сделать сразу)
*/

#define OLD_LOGIC

CyclogramWidget::CyclogramWidget(QWidget* parent):
    QWidget(parent),
    mDiagramSize(0, 0),
    mCurrentCyclogram(Q_NULLPTR)
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
        if (mCurrentCyclogram->state() == Cyclogram::RUNNING)
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
#ifdef OLD_LOGIC
                addCommandOld(dialog->shapeType(), point, dialog->param());
#else
                addCommand(dialog->shapeType(), point, dialog->param());
#endif
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
    //mMovingItem->setPosition(mMovingItem->position() + offset); //TODO item moving disabled
    mPreviousPosition = pos;
    update();
}

ShapeItem* CyclogramWidget::createCommandShape(Command* cmd, const QPoint& cell)
{
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
    shapeItem->setCell(cell);
    shapeItem->setRect(QRect(cell.x(), cell.y(), 1, 1), false); // by initial shape rect matches the occupied cell
    shapeItem->createPath();
    shapeItem->setValencyPoints(createValencyPoints(cmd));
    mCommands.append(shapeItem);

    connect(shapeItem, SIGNAL(changed()), this, SLOT(onNeedUpdate()));

    return shapeItem;
}

ShapeItem* CyclogramWidget::addShape(Command* cmd, const QPoint& cell, ShapeItem* parentShape)
{
    ShapeItem* shapeItem = new ShapeItem(this);
    shapeItem->setCommand(cmd);
    shapeItem->setToolTip(tr("Tooltip"));
    shapeItem->setCell(cell);
    shapeItem->setRect(QRect(cell.x(), cell.y(), 1, 1), false); // by initial shape rect matches the occupied cell
    shapeItem->createPath();
    shapeItem->setValencyPoints(createValencyPoints(cmd));
    shapeItem->setParentShape(parentShape);

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
    QPoint bottomLeft(ShapeItem::origin().x(), mDiagramSize.height() * ShapeItem::itemSize().height());
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

    ShapeItem* sihlouetteItem = new ShapeItem(this);
    sihlouetteItem->setPath(silhouette);
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

#ifdef OLD_LOGIC
    createCommandShape(first, parentCell);
    addChildCommands(first, parentCell);
#else
    ShapeItem* title = addShape(first, parentCell, 0);
    drawCyclogram(title);
    QRect rect = title->rect();
    mRootShape = title;
    mDiagramSize.setWidth(rect.width());
    mDiagramSize.setHeight(rect.height());
#endif
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
            ValencyPoint point = createPoint(QPointF(ShapeItem::itemSize().width() / 2, ShapeItem::itemSize().height() - ShapeItem::cellSize().height() / 2), ValencyPoint::Down);
            points.push_back(point);

            if (type == DRAKON::BRANCH_BEGIN && !isCyclogramEndBranch(cmd))
            {
                ValencyPoint point = createPoint(QPointF(ShapeItem::itemSize().width(), 0), ValencyPoint::Right);
                points.push_back(point);
            }
        }
        break;

    case DRAKON::QUESTION:
        {
            CmdQuestion* questionCmd = qobject_cast<CmdQuestion*>(cmd);
            if (questionCmd)
            {
                ValencyPoint rightPoint = createPoint(QPointF(ShapeItem::itemSize().width(), ShapeItem::itemSize().height() / 2), ValencyPoint::Right);
                points.push_back(rightPoint);

                if (questionCmd->questionType() == CmdQuestion::IF)
                {
                    ValencyPoint downPoint = createPoint(QPointF(ShapeItem::itemSize().width() / 2, ShapeItem::itemSize().height() - ShapeItem::cellSize().height() / 2), ValencyPoint::Down);
                    points.push_back(downPoint);

                    ValencyPoint underArrowPoint = createPoint(QPointF(ShapeItem::itemSize().width() / 2, ShapeItem::itemSize().height() + ShapeItem::cellSize().height() / 2), ValencyPoint::UnderArrow);
                    points.push_back(underArrowPoint);

                }
                else if (questionCmd->questionType() == CmdQuestion::CYCLE)
                {
                    ValencyPoint downPoint = createPoint(QPointF(ShapeItem::itemSize().width() / 2, ShapeItem::itemSize().height()), ValencyPoint::Down);
                    points.push_back(downPoint);

                    ValencyPoint underArrowPoint = createPoint(QPointF(ShapeItem::itemSize().width() / 2, ShapeItem::cellSize().height() / 2), ValencyPoint::UnderArrow);
                    points.push_back(underArrowPoint);
                }
            }
        }
        break;
    default:
        break;
    }

    return points;
}

ValencyPoint CyclogramWidget::createPoint(const QPointF& point, ValencyPoint::Role role)
{
    QPainterPath path;

    qreal crossSize = 0.6;
    qreal radius = qMin(ShapeItem::cellSize().width(), ShapeItem::cellSize().height()) / 3;
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

ShapeItem* CyclogramWidget::addCommandOld(DRAKON::IconType type, const ValencyPoint& point, int param /*= -1*/)
{
    int role = point.role();
    if (type == DRAKON::BRANCH_BEGIN && role == 1) //TODO make enum for roles?
    {
        return addNewBranchOld(point.owner());
    }

    if (type == DRAKON::QUESTION)
    {
        return addQuestion(point, param);
    }

    // 1. Create new command
    Command* newCmd = mCurrentCyclogram->createCommand(type, param);
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
    // create new shape below the points' owner
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
    //qDebug("On need update w=%i, h=%i", mDiagramSize.width(), mDiagramSize.height());

#ifdef OLD_LOGIC
#else
    mDiagramSize.setWidth(mRootShape->rect().width());
    mDiagramSize.setHeight(mRootShape->rect().height());
#endif

    int w = ShapeItem::itemSize().width() * mDiagramSize.width() + ShapeItem::origin().x() + ShapeItem::cellSize().width();
    int h = ShapeItem::itemSize().height() * mDiagramSize.height() + ShapeItem::origin().y() + ShapeItem::cellSize().height();
    resize(w, h);

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

    int TODO5; // make control over command by shape (delete shape = delete command)

    mDiagramSize.setWidth(mDiagramSize.width() - w);

    // kill all shapes and commands, belonging to deleting branch
    ownGoToBranchItem->command()->replaceCommand(Q_NULLPTR); // to block further tree deletion
    mCurrentCyclogram->deleteCommand(item->command(), true);

    onNeedUpdate();
}

ShapeItem* CyclogramWidget::addNewBranchOld(ShapeItem* item)
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
    ShapeItem* goToBranchItem = addCommandOld(DRAKON::GO_TO_BRANCH, newBranchItem->valencyPoint(0));

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
    QPoint cell = item->cell();
    cell.setX(cell.x() + xShift);
    cell.setY(cell.y() + yShift);
    item->setCell(cell);

    QRect rect = item->rect();
    rect.setTop(rect.top() + topShift);
    rect.setBottom(rect.bottom() + bottomShift);
    rect.setLeft(rect.left() + xShift);
    rect.setRight(rect.right() + xShift);
    item->setRect(rect, false);

    if (topShift != bottomShift) // rect size changed, update path
    {
        item->createPath();
    }
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

    default:
        break;
    }

    if (!dialog) // show default dialog
    {
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

ShapeItem* CyclogramWidget::addQuestion(const ValencyPoint& point, int param)
{
    //return Q_NULLPTR; //TODO Temporary

    ValencyPoint::Role role = point.role();
    // 1. Create new command
    Command* newCmd = mCurrentCyclogram->createCommand(DRAKON::QUESTION, param);
    if (!newCmd)
    {
        return Q_NULLPTR;
    }

    ShapeItem* owner = point.owner();
    Command* cmd = owner->command();

    // 2. Update command tree connections
    cmd->insertCommand(newCmd, role);


    /* Закономерности, которые удалось засечь при моделировании:
     * 1. Независимо от типа вопроса ("Условие" или "Цикл"), добавление команды в "правую" точку валентности увеличивает ширину бранча на 1
     * 2. Независимо от типа вопроса ("Условие" или "Цикл"), добавление команды в любую точку валентности увеличивает высоту бранча на 1
     * в тех же ситуациях, что и обычная команда (если в столбце нет "растянутой команды")
     * 3. Независимо от типа вопроса ("Условие" или "Цикл") команда всегда имеет 3 точки валентности (вниз, вправо и "под стрелкой")
     * 4. -||- удаление команды сносит саму команду и все, что идет в ее правую ветку
     *

*/

    /* Как эту поебень отриcовывать?
     *
     * 1. Отрисовыватель шейпа может смотреть на то, где расположен шейп следующей команды
     * 2. По разнице этих ячеек может отрисовваться путь (вниз будет не черточка, а какая-то ломаная линия)
     *
*/

    if (param == CmdQuestion::IF)
    {
        if (cmd->type() == DRAKON::QUESTION)
        {
        }
        else
        {

        }
    }
    else if (param == CmdQuestion::CYCLE)
    {
        if (cmd->type() == DRAKON::QUESTION)
        {

        }
        else
        {

        }
    }

    // 3. Add new command shape item to cyclogram view
    QPoint newCmdCell = owner->cell();

    if (role == ValencyPoint::Down)
    {
        // create new shape below the points' owner
        newCmdCell.setY(newCmdCell.y() + 1);
    }
    else if (role == ValencyPoint::Right)
    {
        // create new shape below and to the right of the the points' owner
        newCmdCell.setY(newCmdCell.y() + 1);
        newCmdCell.setX(newCmdCell.x() + 1); //TODO depends on subtree rect in "down" branch, calculate its rect!

        mDiagramSize.setWidth(mDiagramSize.width() + 1); //TODO unsafe handling
    }
    else if (role == ValencyPoint::UnderArrow)
    {
        int TODO100; // adding under arrow, complex logic here
    }

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
        onNeedUpdate();
    }

    // 5. Update owner rect size recursively (for QUESTION/SWITCH-CASE command trees)
    int TODO2; // owner->parentTreeItem();
    update();

    return newItem;
}

void CyclogramWidget::drawCyclogram(ShapeItem* item)
{
    Command* cmd = item->command();

    if (cmd->type() != DRAKON::TERMINATOR || cmd->nextCommands().empty())
    {
        qDebug("Not cyclogram start terminator");
        return;
    }

    QList<Command*> branches;
    mCurrentCyclogram->getBranches(branches);

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
        drawChildren(shape);
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

void CyclogramWidget::drawChildren(ShapeItem* item)
{
    Command* cmd = item->command();

    if (cmd->childCommands().empty())
    {
        return; // command has no valency points
    }

    if (cmd->childCommands().size() == 1)
    {
        if (cmd->childCommands()[0] == Q_NULLPTR)
        {
            return; // empty valency point
        }

        QPoint cell = item->cell();
        cell.setY(cell.y() + 1);

        ShapeItem* shape = addShape(cmd->childCommands()[0], cell, item);
        item->setChildShape(shape, 0);
        drawChildren(shape);

        QRect rect = item->rect();
        rect.setBottom(rect.bottom() + shape->rect().height());
        rect.setRight(rect.right() + shape->rect().width() - rect.width());
        item->setRect(rect, false);
    }
    else if (cmd->childCommands().size() == 3)
    {
        int i = 0;
    }

    /*

    switch (cmd->type())
    {
    case DRAKON::TERMINATOR:
    case DRAKON::BRANCH_BEGIN:
    case DRAKON::GO_TO_BRANCH:
    case DRAKON::ACTION_MATH:
    case DRAKON::DELAY:
    case DRAKON::QUESTION:
    case DRAKON::ACTION_MODULE:
    case DRAKON::SUBPROGRAM:
    case DRAKON::SWITCH:
    case DRAKON::CASE:
    case DRAKON::FOR_BEGIN:
    case DRAKON::FOR_END:
    case DRAKON::OUTPUT:
    case DRAKON::INPUT:
    case DRAKON::START_TIMER:
    case DRAKON::SYNCHRONIZER:
    case DRAKON::PARALLEL_PROCESS:
    case DRAKON::SHELF:

        break;
    default:
        {

        }
        break;
    }*/

///////////////////
/*    QList<Command*> nextCommands = parentCmd->nextCommands();
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
    }*/
}


ShapeItem* CyclogramWidget::addCommand(DRAKON::IconType type, const ValencyPoint& point, int param /*= -1*/)
{
    int role = point.role();
    if (type == DRAKON::BRANCH_BEGIN && role == ValencyPoint::Right)
    {
        return addNewBranch(point.owner());
    }

    // 1. Create new command
    Command* newCmd = mCurrentCyclogram->createCommand(type, param);
    if (!newCmd)
    {
        return Q_NULLPTR;
    }

    ShapeItem* owner = point.owner();
    Command* cmd = owner->command();

    // 2. Update command tree connections in logic
    if (cmd->type() == DRAKON::BRANCH_BEGIN && newCmd->type() == DRAKON::GO_TO_BRANCH) // new branch creation
    {
        cmd->addCommand(newCmd, role);
    }
    else
    {
        cmd->insertCommand(newCmd, role);
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
        if (cmd->type() == DRAKON::QUESTION)
        {
            int TODO; // depending on role and shape type
        }
        else
        {
            // create new shape below the points' owner
            newCmdCell = owner->cell();
            newCmdCell.setY(newCmdCell.y() + 1);
        }
    }

    // 3.2 create shape
    ShapeItem* newShape = addShape(newCmd, newCmdCell, owner);

    // 3.3 update all rects (child and parent recursively)
    if (prevChildShape)
    {
        // 3.3.1 update shape connections
        newShape->setChildShape(prevChildShape, role);
        owner->setChildShape(newShape, role);

        // 3.3.2 update child rects, then parent rects
        prevChildShape->pushDown();
        QRect rect = prevChildShape->rect();
        rect.setTop(rect.top() - 1);

        newShape->setRect(rect, false);
        owner->onChildRectChanged(newShape);
    }
    else // if owner hadn't any child shapes, update only to parent direction (QUESTION-IF branches ending)
    {
        int TODO1; //QUESTION
        owner->setChildShape(newShape, role);
        owner->onChildRectChanged(newShape);
    }

    update();

    return newShape;
}

ShapeItem* CyclogramWidget::addNewBranch(ShapeItem* item)
{
    // Create and add BRANCH_BEGIN item
    Command* newCmd = mCurrentCyclogram->createCommand(DRAKON::BRANCH_BEGIN);
    if (!newCmd)
    {
        return Q_NULLPTR;
    }

    //generate unique branch name
    CmdStateStart* cmd = qobject_cast<CmdStateStart*>(newCmd);
    cmd->setText(generateBranchName());

    // create new branch to the right of the item command tree
    QPoint newCmdCell = item->cell();
    newCmdCell.setX(newCmdCell.x() + item->rect().width());

    // Update commands positions to the right of the inserted branch
    int xNext = newCmdCell.x();
    foreach (ShapeItem* it, mCommands)
    {
        if (it->cell().x() >= xNext)
        {
            updateItemGeometry(it, 1, 0, 0, 0);
        }
    }

    // update diagram rect
    QRect r = mRootShape->rect();
    r.setRight(r.right() + item->rect().width());
    mRootShape->setRect(r, false);


    ShapeItem* newBranchItem = addShape(newCmd, newCmdCell, mRootShape);
    mRootShape->addChildShape(newBranchItem);

    // Create and add GO_TO_BRANCH item to the created branch (by default new branch is linked to itself)
    ShapeItem* goToBranchItem = addCommand(DRAKON::GO_TO_BRANCH, newBranchItem->valencyPoint(ValencyPoint::Down));
    goToBranchItem->command()->addCommand(newBranchItem->command());

    onNeedUpdate();

    return newBranchItem;
}
