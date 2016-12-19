#include <QtWidgets>

#include "Headers/gui/cyclogram/shape_item.h"
#include "Headers/logic/command.h"

#include "Headers/logic/commands/cmd_title.h"
#include "Headers/logic/commands/cmd_question.h"

/* Обобщенные правила редактирования ДРАКОН-схемы:
 *
 * 1. В схеме есть ТОЧКИ ВАЛЕНТНОСТИ, АТОМЫ и ЛИАНЫ
 * 2. АТОМ - это по сути единичная команда циклограммы (ACTION, QUESTION и т.д.)
 * 3. ТОЧКА ВАЛЕНТНОСТИ - это место в циклограмме, куда могут цепляться АТОМЫ
 * 4. ТОЧКИ ВАЛЕНТНОСТИ по сути принадлежат какому-о из атомов
 * 5. Каждая ТОЧКА ВАЛЕНТНОСТИ содержит "матрешку" команд
 * 6. Матрешка обладает RECT'ом (левый верхний угол + правый нижний)
 * 7. К каждой точке валентности привязана такая "матрешка"
 * 8. При вставке в точку валентности новой "матрешки" у нее обновляется RECT
 * 9. После этого рекурсивно обновляется RECT парентовой "матрешки" и т.д. до самой верхней матрешки
 * 10. Парентовая матрешка при этом следит за RECTами чайлдовыйх матрешек
 * 11. Если это УСЛОВИЕ, то ректы "вниз" и "вправо" должны совпадать по высоте
*/

namespace
{
    static const int CELL_WIDTH = 30;
    static const int CELL_HEIGHT = 30;
    static const int CELLS_PER_ITEM_V = 4;
    static const int CELLS_PER_ITEM_H = 8;
}

ShapeItem::ShapeItem(QObject* parent):
    QObject(parent),
    mCommand(Q_NULLPTR),
    mParentShape(Q_NULLPTR),
    mActive(false),
    mPosition(QPoint(0, 0)),
    mCell(QPoint(0, 0)),
    mRect(QRect(0, 0, 0, 0))
{
    mFont.setPointSize(14);
    mFont.setFamily("Arial");

    mActiveColor = QColor::fromRgba(0xff7f7f7f);
    mAdditionalColor = QColor::fromRgba(0x00ffffff);
}

QPainterPath ShapeItem::path() const
{
    return mPath;
}

QPainterPath ShapeItem::textPath() const
{
    return mTextPath;
}

QPainterPath ShapeItem::additionalPath() const
{
    return mAdditionalPath;
}

QPainterPath ShapeItem::arrowPath() const
{
    return mArrowPath;
}

QPoint ShapeItem::position() const
{
    return mPosition;
}

QColor ShapeItem::color() const
{
    if (mActive)
    {
        return mActiveColor;
    }

    return mColor;
}

QColor ShapeItem::additionalColor() const
{
    return mAdditionalColor;
}

QString ShapeItem::toolTip() const
{
    return mToolTip;
}

void ShapeItem::setPath(const QPainterPath &path)
{
    mPath = path;
}

void ShapeItem::setTextPath(const QPainterPath &path)
{
    mTextPath = path;
}

void ShapeItem::setToolTip(const QString &toolTip)
{
    mToolTip = toolTip;
}

void ShapeItem::setColor(const QColor &color)
{
    mColor = color;
}

QPoint ShapeItem::cell() const
{
    return mCell;
}

void ShapeItem::setCell(const QPoint &cell)
{
    mCell = cell;
    mPosition.setX(origin().x() + mCell.x() * itemSize().width());
    mPosition.setY(origin().y() + mCell.y() * itemSize().height());
}

void ShapeItem::setCommand(Command* command)
{
    if (mCommand)
    {
        disconnect(mCommand, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
        disconnect(mCommand, SIGNAL(errorStatusChanged(bool)), this, SLOT(onErrorStatusChanged(bool)));
        disconnect(mCommand, SIGNAL(activeStateChanged(bool)), this, SLOT(setActive(bool)));
    }

    mCommand = command;
    connect(mCommand, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
    connect(mCommand, SIGNAL(errorStatusChanged(bool)), this, SLOT(onErrorStatusChanged(bool)));
    connect(mCommand, SIGNAL(activeStateChanged(bool)), this, SLOT(setActive(bool)));

    onTextChanged(mCommand->text());
    onErrorStatusChanged(mCommand->hasError());

    mChildShapes.clear();
    for (int i = 0; i < mCommand->childCommands().size(); ++i)
    {
        mChildShapes.push_back(Q_NULLPTR);
    }
}

Command* ShapeItem::command() const
{
    return mCommand;
}

const QList<ValencyPoint>& ShapeItem::valencyPoints() const
{
    return mValencyPoints;
}

void ShapeItem::setValencyPoints(const QList<ValencyPoint>& points)
{
    mValencyPoints = points;
    for (int i = 0, sz = mValencyPoints.size(); i < sz; ++i)
    {
        mValencyPoints[i].setOwner(this);
    }
}

ValencyPoint ShapeItem::valencyPoint(int role) const
{
    for (int i = 0, sz = mValencyPoints.size(); i < sz; ++i)
    {
        if (role == mValencyPoints[i].role())
        {
            return mValencyPoints[i];
        }
    }

    int TODO; // make point not found notification
    return ValencyPoint();
}

void ShapeItem::setRect(const QRect& rect, bool pushToChildren)
{
    if (mRect == rect)
    {
        return; // rect doesn't changed
    }

    if (pushToChildren)
    {
        if (mCommand)
        {
            const QList<Command*>& childs = mCommand->childCommands();

            if (childs.empty())
            {
                QPoint cell = mCell;
                cell.setY(cell.y() + rect.height() - mRect.height()); // update cell first
                setCell(cell);
                mRect.setBottom(mRect.bottom() + rect.height() - mRect.height()); // then update rect
            }
            else if (childs.size() == 1)
            {
                if (childs[0] != Q_NULLPTR)
                {
                    // has one possible child command
                    QRect newRect = rect;
                    newRect.setTop(newRect.top() + 1);
                    mChildShapes[0]->setRect(newRect, true);
                }
            }
            else if (childs.size() == 3)
            {
                int TODO; // QUESTION

                // has 3 possible child commands
                if (mCommand->type() == DRAKON::QUESTION)
                {
                    CmdQuestion* question = qobject_cast<CmdQuestion*>(mCommand);
                    if (question->questionType() == CmdQuestion::CYCLE)
                    {

                    }
                    else // IF-type
                    {
                        if (childs[ValencyPoint::UnderArrow] == Q_NULLPTR)
                        {// "landed" branches

                        }
                        else
                        {

                        }
                    }
                }
            }
        }
    }

    mRect = rect;
    createPath();
}

QRect ShapeItem::rect() const
{
    return mRect;
}

void ShapeItem::setParentShape(ShapeItem* parent)
{
    mParentShape = parent;
}

ShapeItem* ShapeItem::parentShape() const
{
    return mParentShape;
}

ShapeItem* ShapeItem::childShape(int index) const
{
    if (index >= 0 && index < mChildShapes.size())
    {
        return mChildShapes[index];
    }

    return Q_NULLPTR;
}

void ShapeItem::setChildShape(ShapeItem* item, int index)
{
    if (index >= 0 && index < mChildShapes.size())
    {
        item->setParentShape(this);
        mChildShapes[index] = item;
    }
}

void ShapeItem::addChildShape(ShapeItem* item)
{
    mChildShapes.push_back(item);
}

void ShapeItem::removeChildShape(ShapeItem* item)
{
    mChildShapes.removeAll(item);
}

void ShapeItem::onTextChanged(const QString& text)
{
    QPainterPath textPath;
    QFontMetrics fm(mFont);
    QRect textRect = fm.boundingRect(text);
    qreal x = (itemSize().width() - textRect.width()) / 2;
    qreal y = (itemSize().height() + textRect.height()) / 2;
    textPath.addText(x, y, mFont, text);
    mTextPath = textPath;

    if (mCommand && mCommand->type() == DRAKON::QUESTION)
    {
        CmdQuestion* cmd = qobject_cast<CmdQuestion*>(command());
        if (cmd)
        {
            bool yesDown = (cmd->orientation() == CmdQuestion::YesDown);

            QPainterPath additionalText;
            QFont font;
            font.setPointSize(10);
            font.setFamily("Arial");

            qreal x1 = itemSize().width() - cellSize().width();
            qreal y1 = itemSize().height() / 2 - cellSize().width() * 0.1;
            additionalText.addText(x1, y1, font, yesDown ? tr("No") : tr("Yes"));

            qreal x2 = itemSize().width() / 2 + cellSize().width() / 3;
            qreal y2 = itemSize().height() - cellSize().height() * 0.6;
            additionalText.addText(x2, y2, font, yesDown ? tr("Yes") : tr("No"));

            mTextPath.addPath(additionalText);
        }
    }

    emit changed();
}

void ShapeItem::onErrorStatusChanged(bool status)
{
    setColor(status ? QColor::fromRgba(0xffff0000) : QColor::fromRgba(0xffffffff));
}

const QSizeF& ShapeItem::itemSize()
{
    static QSizeF ITEM_SIZE(CELL_WIDTH * CELLS_PER_ITEM_H, CELL_HEIGHT * CELLS_PER_ITEM_V);
    return ITEM_SIZE;
}

const QSizeF& ShapeItem::cellSize()
{
    static QSizeF CELL(CELL_WIDTH, CELL_HEIGHT);
    return CELL;
}

const QPointF& ShapeItem::origin()
{
    static QPointF ORIGIN(itemSize().width() / 4, 0);
    return ORIGIN;
}

void ShapeItem::setActive(bool active)
{
    mActive = active;
    emit changed();
}

void ShapeItem::createPath()
{
    DRAKON::IconType type = command()->type();
    QPainterPath path;

    QPoint cell = mCell;

    qreal W = itemSize().width();
    qreal H = itemSize().height();
    qreal w = cellSize().width();
    qreal h = cellSize().height();

    switch (type)
    {
    case DRAKON::TERMINATOR:
        {
            qreal yOffset = (cell.y() - mRect.top()) * H;
            qreal radius = (H - 2 * h) / 2;
            QRectF rect(w, h, W - 2 * w, 2 * radius);
            path.addRoundedRect(rect, radius, radius);

            // connector
            CmdTitle* titleCmd = qobject_cast<CmdTitle*>(command());
            if (titleCmd)
            {
                if (titleCmd->titleType() == CmdTitle::BEGIN)
                {
                    path.moveTo(W / 2, H - h);
                    path.lineTo(W / 2, H);
                }
                else // END terminator
                {
                    path.moveTo(W / 2, h);
                    path.lineTo(W / 2, -yOffset);
                }
            }
        }
        break;
    case DRAKON::BRANCH_BEGIN:
        {
            path.moveTo(w, h);
            path.lineTo(w, H - h * 3 / 2);
            path.lineTo(W / 2, H - h);
            path.lineTo(W - w, H - h * 3 / 2);
            path.lineTo(W - w, h);
            path.lineTo(w, h);
        }
        break;
    case DRAKON::GO_TO_BRANCH:
        {
            path.moveTo(w, h * 3 / 2);
            path.lineTo(w, H - h);
            path.lineTo(W - w, H - h);
            path.lineTo(W - w, h * 3 / 2);
            path.lineTo(W / 2, h);
            path.lineTo(w, h * 3 / 2);
        }
        break;
    case DRAKON::ACTION_MATH:
    case DRAKON::ACTION_MODULE:
        {
            path.addRect(QRect(w, h, W - 2 * w, H - 2 * h));
        }
        break;
    case DRAKON::DELAY:
        {
            path.moveTo(w, h);
            path.lineTo(w * 2, H - h);
            path.lineTo(W - 2 * w, H - h);
            path.lineTo(W - w, h);
            path.lineTo(w, h);
        }
        break;

    case DRAKON::QUESTION:
        {
            CmdQuestion * questionCmd = qobject_cast<CmdQuestion*>(command());

            if (questionCmd)
            {
                int TODO; // very complex logics for QUESTION connections drawing will be here

                path.moveTo(w, H / 2);
                path.lineTo(w * 2, H - h);
                path.lineTo(W - 2 * w, H - h);
                path.lineTo(W - w, H / 2);
                path.lineTo(W - 2 * w, h);
                path.lineTo(w * 2, h);
                path.lineTo(w, H / 2);

                QPainterPath addPath;
                if (questionCmd->questionType() == CmdQuestion::IF)
                {
                    addPath.moveTo(W - w, H / 2);
                    addPath.lineTo(W, H / 2);
                    addPath.lineTo(W, H);
                    addPath.lineTo(W / 2, H);
                }
                else if (questionCmd->questionType() == CmdQuestion::CYCLE)
                {
                    addPath.moveTo(W - w, H / 2);
                    addPath.lineTo(W, H / 2);
                    addPath.lineTo(W, 0);
                    addPath.lineTo(W / 2, 0);

                    QPainterPath arrowPath;
                    QPoint pos(W / 2, 0);
                    arrowPath.moveTo(pos);
                    arrowPath.lineTo(QPoint(pos.x() + w, pos.y() + h / 4));
                    arrowPath.lineTo(QPoint(pos.x() + w, pos.y() - h / 4));
                    arrowPath.lineTo(pos);
                    mArrowPath = arrowPath;
                }

                mAdditionalPath = addPath;
            }
        }
        break;
    default:
        break;
    }

    if (type != DRAKON::TERMINATOR)
    {
        qreal yOffset = (cell.y() - mRect.top()) * H;

        // lower connector
        path.moveTo(W / 2, H - h);
        path.lineTo(W / 2, H);
        // upper connector
        path.moveTo(W / 2, h);
        path.lineTo(W / 2, -yOffset);
    }

    mPath = path;
}

void ShapeItem::setSelected(bool selected)
{
    if (selected)
    {
        setColor(QColor::fromRgba(0xff00ff00));
    }
    else
    {
        onErrorStatusChanged(mCommand->hasError());
    }
}

void ShapeItem::pushDown()
{
    if (mChildShapes.empty() || (mChildShapes.size() == 1 && mChildShapes[0] == Q_NULLPTR))
    {
        if (mRect.height() > 1) // "expanded shape"
        {
            // just reduce own rect height by 1
            mRect.setTop(mRect.top() + 1);
        }
        else // move shape down
        {
            mRect.setBottom(mRect.bottom() + 1);
            mRect.setTop(mRect.top() + 1);

            QPoint cell = mCell;
            cell.setY(cell.y() + 1);
            setCell(cell);
        }
    }
    else if (mChildShapes.size() == 1)
    {
        QPoint cell = mCell;
        cell.setY(cell.y() + 1);
        setCell(cell);

        // tell child to push itself down
        mChildShapes[0]->pushDown();

        QRect rect = mChildShapes[0]->rect();
        rect.setTop(rect.top() - 1);
        setRect(rect, false);
    }
    else if (mChildShapes.size() == 3)
    {
        int TODO5; // question logic
    }

    createPath();
}

void ShapeItem::onChildRectChanged(ShapeItem * shape)
{
    if (mCommand && mCommand->type() == DRAKON::TERMINATOR && !mCommand->nextCommands().empty())
    {
        updateCyclogramRect(shape);
        return;
    }

    if (mChildShapes.size() == 1)
    {
        QRect rect = shape->rect();
        rect.setTop(rect.top() - 1);
        setRect(rect, false);

        if (mParentShape)
        {
            mParentShape->onChildRectChanged(this);
        }
    }
    else if (mChildShapes.size() == 3)
    {
        int TODO; // custom logic for question
    }
}

void ShapeItem::updateCyclogramRect(ShapeItem* changedBranch)
{
    int TODO; // branch deletion

    // 1. Сюда мы попадаем, когда меняется размер какого-то бранча (а может де-факто не поменяться теоретически)
    // 2. Сюда мы можем попасть при добавлении команды
    // 3. Если совкупная ширина бранчей стала больше ректа this'а (добавилась новая команда в какой-то из боковых веток), то ширина поменялась, this-у обновляем рект
    // 4. Пробегаем по всем бранчам (сортированным слева направо) смотрим cell и width его ректа
    // 5. Если находим пересечение, то все бранчи начиная с пересекаемого и правее двигаем на дельту-width вправо (например switch-case могли добавить)
    // 6. Далее смотрим поменялась ли высота бранча
    // 7. Если бранч стал выше ректа this'a, то считаем, что бранч удлинился и пушим новый рект в остальные бранчи (где-то подрастянется), this-у обновляем рект
    // 8. Если бранч стал ниже ректа this'а, то проводим тест по другим бранчам на предмет "можно ли их ужать"
    // 9. Если все бранчи можно ужать до новой высоты, то пушим в них новые ректы и обновляем рект this'а
    // 10. Если хоть один бранч ужать нельзя, то бранчу, которого поменялась высота ректа пушим текущую высоту this'а

    QRect before = mRect;

    //1. If branch width changed, shift all branches to the right of changed
    int xOffset = -mRect.width();
    foreach (ShapeItem* it, mChildShapes)
    {
        xOffset += it->rect().width();
    }

    if (xOffset != 0)
    {
        QRect rect = changedBranch->rect();

        foreach (ShapeItem* it, mChildShapes)
        {
            QRect branchRect = it->rect();
            if (branchRect.left() > rect.left())
            {
                branchRect.setLeft(branchRect.left() + xOffset);
                branchRect.setRight(branchRect.right() + xOffset);
                it->setRect(branchRect, true);
            }
        }

        mRect.setRight(mRect.right() + xOffset);
    }

    int yOffset = (changedBranch->rect().height() + 1) - mRect.height();

    if (yOffset != 0)
    {
        bool canSetRect = true;

        foreach (ShapeItem* it, mChildShapes)
        {
            if (it != changedBranch)
            {
                QRect branchRect = it->rect();
                branchRect.setBottom(branchRect.bottom() + yOffset);

                if (!it->canSetRect(branchRect))
                {
                    canSetRect = false;
                    break;
                }
            }
        }

        if (canSetRect) // push new size to all other branches
        {
            foreach (ShapeItem* it, mChildShapes)
            {
                if (it != changedBranch)
                {
                    QRect branchRect = it->rect();
                    branchRect.setBottom(branchRect.bottom() + yOffset);
                    it->setRect(branchRect, true);
                }
            }

            mRect.setBottom(mRect.bottom() + yOffset);
        }
        else
        {// find the highest branch, and set rect for all branches
            int minHeight = 0;
            foreach (ShapeItem* it, mChildShapes)
            {
                if (it->minHeight() > minHeight)
                {
                    minHeight = it->minHeight();
                }
            }

            foreach (ShapeItem* it, mChildShapes)
            {
                QRect branchRect = it->rect();
                branchRect.setBottom(branchRect.bottom() + minHeight - branchRect.height());
                it->setRect(branchRect, true);
            }

            mRect.setBottom(mRect.bottom() + minHeight - mRect.height() + 1);
        }
    }

    if (before != mRect)
    {
        emit changed();
    }
}

bool ShapeItem::canSetRect(const QRect& rect) const
{
    if (mRect.top() == rect.top() && mRect.left() == rect.left() && mRect.right() == rect.right())
    {
        if (mRect.bottom() <= rect.bottom())
        {
            return true;
        }

        if (mChildShapes.empty() || (mChildShapes.size() == 1 && mChildShapes[0] == Q_NULLPTR))
        {
            return (mRect.bottom() >= rect.bottom());
        }
        else if (mChildShapes.size() == 1)
        {
            QRect tempRect = rect;
            tempRect.setTop(tempRect.top() + 1);

            if (tempRect.top() == tempRect.bottom())
            {
                return false;
            }

            return mChildShapes[0]->canSetRect(tempRect);
        }
        else if (mChildShapes.size() == 3) //
        {
            int TODO; // question
        }
    }

    return false;
}

int ShapeItem::minHeight() const
{
    int minHeight = 1;

    if (mChildShapes.empty() || (mChildShapes.size() == 1 && mChildShapes[0] == Q_NULLPTR))
    {
        return minHeight;
    }

    if (mChildShapes.size() == 1)
    {
        minHeight += mChildShapes[0]->minHeight();
    }
    else if (mChildShapes.size() == 3) //
    {
        int TODO; // question
    }

    return minHeight;
}

void ShapeItem::remove()
{
    if (mChildShapes.empty() || (mChildShapes.size() == 1 && mChildShapes[0] == Q_NULLPTR))
    {
        mRect.setBottom(mRect.top());
        mParentShape->replaceChildShape(0, this); // update shape connections

        mParentShape->onChildRectChanged(this);
        return;
    }

    if (mChildShapes.size() == 1)
    {
        ShapeItem* item = mChildShapes[0];

        mParentShape->replaceChildShape(item, this); // update shape connections
        item->setParentShape(mParentShape);

        item->pullUp();
        mParentShape->onChildRectChanged(item);
    }
    else if (mChildShapes.size() == 3) //
    {
        int TODO; // question
    }
}

void ShapeItem::replaceChildShape(ShapeItem* newItem, ShapeItem* oldItem)
{
    for (int i = 0, sz = mChildShapes.size(); i < sz; ++i)
    {
        if (mChildShapes[i] == oldItem)
        {
            if (newItem == Q_NULLPTR)
            {
                mCommand->replaceChildCommand(0, oldItem->command());
            }

            mChildShapes[i] = newItem;
            return;
        }
    }
}

void ShapeItem::pullUp()
{
    if (mChildShapes.empty() || (mChildShapes.size() == 1 && mChildShapes[0] == Q_NULLPTR))
    {
        // just move shape up
        mRect.setTop(mRect.top() - 1);
        mRect.setBottom(mRect.bottom() - 1);

        QPoint cell = mCell;
        cell.setY(cell.y() - 1);
        setCell(cell);
    }
    else if (mChildShapes.size() == 1)
    {
        QPoint cell = mCell;
        cell.setY(cell.y() - 1);
        setCell(cell);

        // tell child to pull itself up
        mChildShapes[0]->pullUp();

        QRect rect = mChildShapes[0]->rect();
        rect.setTop(rect.top() - 1);
        setRect(rect, false);
    }
    else if (mChildShapes.size() == 3)
    {
        int TODO5; // question logic
    }

    createPath();
}
