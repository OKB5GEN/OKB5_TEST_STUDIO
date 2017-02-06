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
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/logic/commands/cmd_delay.h"
#include "Headers/logic/commands/cmd_state_start.h"
#include "Headers/logic/commands/cmd_set_state.h"
#include "Headers/logic/commands/cmd_question.h"

#include "Headers/gui/cyclogram/dialogs/cmd_action_math_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_action_module_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_delay_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_state_start_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_set_state_edit_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_question_edit_dialog.h"
#include "Headers/gui/cyclogram/shape_item.h"

#include "Headers/logger/Logger.h"

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

CyclogramWidget::CyclogramWidget(QWidget* parent):
    QWidget(parent),
    mCurrentCyclogram(Q_NULLPTR),
    mSihlouetteLine(Q_NULLPTR),
    mSihlouetteArrow(Q_NULLPTR),
    mUpdateOnShapeRemove(true)
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
    mCommands.clear();

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
        //LOG_INFO("Mouse clicked");

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

    ShapeItem* title = addShape(first, QPoint(0, 0), 0);
    mRootShape = title;
    drawCyclogram(title);

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
        if (startBranch->nextCommand() == item->command())
        {
            error = tr("Start branch never can be deleted");
            return false;
        }

        const ShapeItem* lastBranch = findBranch(mCurrentCyclogram->last());

        // check is end branch trying to delete
        if (lastBranch->command() == item->command())
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
            const ShapeItem* currentBranch = findBranch(mCurrentCyclogram->current());

            if (currentBranch->command() == item->command())
            {
                error = tr("Has running command in this branch");
                return false;
            }
        }
    }

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

    if (mUpdateOnShapeRemove)
    {
        onNeedUpdate();
    }
}

void CyclogramWidget::onNeedToDelete(ShapeItem* shape)
{
    mCurrentCyclogram->deleteCommand(shape->command());
}

void CyclogramWidget::onNeedUpdate()
{
    //qDebug("On need update w=%i, h=%i", mDiagramSize.width(), mDiagramSize.height());

    int W = ShapeItem::itemSize().width();
    int H = ShapeItem::itemSize().height();
    int w = ShapeItem::cellSize().width();
    int h = ShapeItem::cellSize().height();

    int width = W * mRootShape->rect().width() + ShapeItem::origin().x() + w;
    int height = H * mRootShape->rect().height() + ShapeItem::origin().y() + h;
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
            QString msg = QString("Stopping draw chidren: Cmd:%1 Next:%2 ").arg(cmd->text()).arg(nextCmd->text());
            LOG_DEBUG(msg);
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
    Command* newCmd = mCurrentCyclogram->createCommand(type, param);
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
    mCurrentCyclogram->deleteCommand(item->command()); // shape will be deleted by the signal
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
    mCurrentCyclogram->deleteCommand(item->command(), true);

    QRect r = mRootShape->rect();
    r.setRight(r.right() + xOffset);
    mRootShape->setRect(r, false);

    // 5. Update entire cyclogram rect
    mRootShape->adjust();
}

void CyclogramWidget::setUpdateOnRemove(bool updateOnShapeRemove)
{
    mUpdateOnShapeRemove = updateOnShapeRemove;
}
