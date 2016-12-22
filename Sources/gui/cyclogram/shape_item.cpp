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
    if (mCommand->type() != DRAKON::TERMINATOR && mCommand->type() != DRAKON::GO_TO_BRANCH)
    {
        for (int i = 0; i < mCommand->nextCommands().size(); ++i)
        {
            mChildShapes.push_back(Q_NULLPTR);
        }
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
//    if (mRect == rect)
//    {
//        return; // rect doesn't changed, BUT its child shapes rect can be chaged
//    }

    if (pushToChildren)
    {
        int xOffset = rect.right() - mRect.right();
        QPoint cell = mCell;
        cell.setX(cell.x() + xOffset);
        cell.setY(cell.y() + rect.bottom() - mRect.bottom()); // place own shape to the bottom of its rect by default

        const QList<Command*>& commands = mCommand->nextCommands();

        if (commands.size() == 1 && mCommand->type() != DRAKON::GO_TO_BRANCH && mCommand->nextCommand())
        {
            ShapeItem* down = mChildShapes[ValencyPoint::Down];
            if (down)
            {
                // if has child shapes below, place own shape to the top of its rect
                cell.setY(mCell.y() + rect.top() - mRect.top());

                // if has shape below, reduce rect by 1 (own shape height), and push rect further down
                QRect newRect = rect;
                newRect.setTop(newRect.top() + 1);
                down->setRect(newRect, true);
            }
        }
        else if (commands.size() == 3)
        {
            int yOffset = rect.top() - mRect.top();
            cell.setY(mCell.y() + yOffset); // if has child shapes below, place own shape to the top of its rect

            CmdQuestion* question = qobject_cast<CmdQuestion*>(mCommand);
            if (question->questionType() == CmdQuestion::CYCLE)
            {
                int i = 0;
                int TODO;
            }
            else // QUESTION-IF
            {
                ShapeItem* underArrow = mChildShapes[ValencyPoint::UnderArrow];
                ShapeItem* down = mChildShapes[ValencyPoint::Down];
                ShapeItem* right = mChildShapes[ValencyPoint::Right];

                if (underArrow)
                {
                    // move "down" and "right" branches without changing their size
                    int branchesHeight = 0;
                    if (down)
                    {
                        QRect downRect = down->rect();
                        downRect.setTop(downRect.top() + yOffset);
                        downRect.setBottom(downRect.bottom() + yOffset);
                        downRect.setLeft(downRect.left() + xOffset);
                        downRect.setRight(downRect.right() + xOffset);
                        down->setRect(downRect, true);

                        branchesHeight = down->rect().height();
                    }

                    if (right)
                    {
                        QRect rightRect = right->rect();
                        rightRect.setTop(rightRect.top() + yOffset);
                        rightRect.setBottom(rightRect.bottom() + yOffset);
                        rightRect.setLeft(rightRect.left() + xOffset);
                        rightRect.setRight(rightRect.right() + xOffset);
                        right->setRect(rightRect, true);

                        branchesHeight = right->rect().height();
                    }

                    // push remainig height to "under arrow" branch, possibly changing its size
                    QRect newRect = rect;
                    newRect.setTop(newRect.top() + branchesHeight + 1); // reduce by own shape height + branches height
                    newRect.setWidth(underArrow->rect().width()); // reduce width to underArrow own width
                    underArrow->setRect(newRect, true);
                }
                else // push rect to both down and right branches (they MUST exist if there is no "under arrow" branch)
                {
                    QRect downRect = rect;
                    downRect.setTop(rect.top() + 1);
                    downRect.setWidth(down->rect().width());
                    down->setRect(downRect, true);

                    QRect rightRect = rect;
                    rightRect.setTop(rect.top() + 1);
                    rightRect.setLeft(rect.left() + downRect.width());
                    right->setRect(rightRect, true);
                }
            }
        }

        setCell(cell);
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
                    ShapeItem* down = mChildShapes[ValencyPoint::Down];
                    ShapeItem* right = mChildShapes[ValencyPoint::Right];
                    ShapeItem* underArrow = mChildShapes[ValencyPoint::UnderArrow];

                    QRect downRect;
                    QRect rightRect;
                    int xOffset = W;

                    if (down)
                    {
                        downRect = down->rect();
                        xOffset += W * (downRect.width() - 1);
                    }

                    if (right)
                    {
                        rightRect = right->rect();
                        xOffset += W / 2;
                    }

                    int yOffset = H * (qMax(downRect.height(), rightRect.height()) + 1);

                    // if underArrow && !down && !right, i.e. by default
                    addPath.moveTo(W - w, H / 2);
                    addPath.lineTo(xOffset, H / 2);
                    addPath.lineTo(xOffset, H);

                    if (!right && down)
                    {
                        addPath.lineTo(xOffset, yOffset);
                    }

                    if (right && !down)
                    {
                        addPath.moveTo(W / 2, H);
                        addPath.lineTo(W / 2, yOffset);
                    }

                    if (underArrow)
                    {
                        addPath.moveTo(xOffset, yOffset);
                        addPath.lineTo(W / 2, yOffset);
                    }
                    else
                    {
                        int i = 0;
                        qDebug("IMPOSSIBRU!!!11");
                    }

                    // update valency point positions
                    for (int i = 0, sz = mValencyPoints.size(); i < sz; ++i)
                    {
                        int role = mValencyPoints[i].role();

                        if (role == ValencyPoint::Right)
                        {
                            mValencyPoints[i] = createValencyPoint(QPointF(xOffset, H / 2), ValencyPoint::Right);
                        }
                        else if (role == ValencyPoint::UnderArrow)
                        {
                            if (underArrow)
                            {
                                mValencyPoints[i] = createValencyPoint(QPointF(W / 2, yOffset + h / 2), ValencyPoint::UnderArrow);
                            }
                            else
                            {
                                int j = 0; // TODO remove valency point? make null path?
                                int TODO;
                            }
                        }
                    }
                }
                else if (questionCmd->questionType() == CmdQuestion::CYCLE)
                {
                    int TODO; // very complex logics for QUESTION-CYCLE connections drawing will be here

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

    if (mValencyPoints.empty())
    {
        createValencyPoints(mCommand);
    }
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
    QRect rect = mRect;

    if (mChildShapes.empty() || (mChildShapes.size() == 1 && mChildShapes[0] == Q_NULLPTR))
    {
        if (mRect.height() > 1) // "expanded shape", just reduce own rect height by 1
        {
            rect.setTop(mRect.top() + 1);
        }
        else // move shape down
        {
            rect.setBottom(rect.bottom() + 1);
            rect.setTop(rect.top() + 1);

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

        rect = mChildShapes[0]->rect();
        rect.setTop(rect.top() - 1);
    }
    else if (mChildShapes.size() == 3)
    {
        int i = 0;
        int TODO5; // question logic
    }

    setRect(rect, false);
}

void ShapeItem::onChildRectChanged(ShapeItem * shape)
{
    if (mCommand && mCommand->type() == DRAKON::TERMINATOR && mCommand->nextCommand())
    {
        updateCyclogramRect(shape);
        return;
    }

    if (mChildShapes.size() == 1)
    {
        QRect rect = shape->rect();
        rect.setTop(rect.top() - 1);
        setRect(rect, false);
    }
    else if (mChildShapes.size() == 3)
    {
        CmdQuestion* cmd = qobject_cast<CmdQuestion*>(mCommand);

        if (cmd->questionType() == CmdQuestion::CYCLE)
        {
            if (mChildShapes[ValencyPoint::UnderArrow] == shape)
            {
                int i = 0;
                int TODO;
            }
            else if (mChildShapes[ValencyPoint::Down] == shape)
            {
                int i = 0;
                int TODO;
            }
            else if (mChildShapes[ValencyPoint::Right] == shape)
            {
                int i = 0;
                int TODO;
            }
        }
        else // QUESTION-IF
        {
            ShapeItem* down = mChildShapes[ValencyPoint::Down];
            ShapeItem* right = mChildShapes[ValencyPoint::Right];
            ShapeItem* underArrow = mChildShapes[ValencyPoint::UnderArrow];

            QRect downRect = down ? down->rect() : QRect();
            QRect rightRect = right ? right->rect() : QRect();
            QRect underArrowRect = underArrow ? underArrow->rect() : QRect();

            if (shape == right || shape == down)
            {
                ShapeItem* branchToAdjust = (shape == down) ? right : down;

                if (branchToAdjust) // both down and right branches exist
                {
                    QRect shapeRect = shape->rect();
                    QRect branchRect = branchToAdjust->rect();

                    // shaif right branch horizontally
                    int xOffset = 0;
                    if (branchRect.left() > shapeRect.left())
                    {
                        if (shapeRect.right() < (branchRect.left() - 1))
                        {
                            xOffset = shapeRect.right() - branchRect.left() + 1; // < 0
                        }
                        else if (shapeRect.right() >= branchRect.left())
                        {
                            xOffset = branchRect.left() - shapeRect.right() + 1; // > 0
                        }
                    }

                    // set min equal height to both branches
                    int minHeight = qMax(shape->minHeight(), branchToAdjust->minHeight());

                    branchRect.setBottom(branchRect.bottom() + minHeight - branchRect.height());
                    shapeRect.setBottom(shapeRect.bottom() + minHeight - shapeRect.height());

                    branchRect.setRight(branchRect.right() + xOffset);
                    branchRect.setLeft(branchRect.left() + xOffset);

                    branchToAdjust->setRect(branchRect, true);
                    shape->setRect(shapeRect, true);

                    // update rects after abjusting
                    downRect = down ? down->rect() : QRect();
                    rightRect = right ? right->rect() : QRect();
                }
                else if (right)
                {
                    // shift right branch to the left if down branch became empty
                    int xOffset = mCell.x() - rightRect.left() + 1;
                    rightRect.setRight(rightRect.right() + xOffset);
                    rightRect.setLeft(rightRect.left() + xOffset);
                    rightRect.setBottom(rightRect.bottom() + right->minHeight() - rightRect.height());
                    right->setRect(rightRect, true);
                }

                if (underArrow) // is changed size of one of branches, move up/down "underArrow" part
                {
                    int yOffset = shape->rect().bottom() - underArrowRect.top() + 1;
                    int delta = underArrow->minHeight() - underArrowRect.height();

                    underArrowRect.setBottom(underArrowRect.bottom() + yOffset + delta);
                    underArrowRect.setTop(underArrowRect.top() + yOffset/* + delta*/);
                    underArrow->setRect(underArrowRect, true);
                }
            }

            QRect newRect = downRect.united(rightRect);
            newRect = newRect.united(underArrowRect);
            newRect.setTop(mRect.top());
            setRect(newRect, false);
        }
    }

    if (mParentShape)
    {
        mParentShape->onChildRectChanged(this);
    }
}

void ShapeItem::adjust()
{
    if (!mCommand || mCommand->type() != DRAKON::TERMINATOR || !mCommand->nextCommand())
    {
        qDebug("Adjust operation is not applicable for this shape");
        return;
    }

    // find the highest branch, and set rect for all branches
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
    emit changed();
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
            if (changedBranch != it && changedBranch->cell().x() <= it->cell().x())
            {
                QRect branchRect = it->rect();
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
        {
            adjust();
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
        minHeight += mChildShapes[ValencyPoint::Down]->minHeight();
    }
    else if (mChildShapes.size() == 3) // QUESTION
    {
        ShapeItem* down = mChildShapes[ValencyPoint::Down];
        ShapeItem* right = mChildShapes[ValencyPoint::Right];
        ShapeItem* underArrow = mChildShapes[ValencyPoint::UnderArrow];

        int minDownHeight = (down ? down->minHeight() : 0);
        int minRightHeight = (right ? right->minHeight() : 0);
        int minUnderArrowHeight = (underArrow ? underArrow->minHeight() : 0);

        minHeight += qMax(minDownHeight, minRightHeight);
        minHeight += minUnderArrowHeight;
    }

    return minHeight;
}

void ShapeItem::remove()
{
    if (mChildShapes.empty() || (mChildShapes.size() == 1 && mChildShapes[ValencyPoint::Down] == Q_NULLPTR))
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
        int i = 0;
        int TODO; // question
    }
}

void ShapeItem::replaceChildShape(ShapeItem* newItem, ShapeItem* oldItem)
{
    for (int i = 0, sz = mChildShapes.size(); i < sz; ++i)
    {
        if (mChildShapes[i] == oldItem)
        {
            mChildShapes[i] = newItem;
            return;
        }
    }
}

void ShapeItem::pullUp()
{
    QRect rect = mRect;
    if (mChildShapes.empty() || (mChildShapes.size() == 1 && mChildShapes[0] == Q_NULLPTR))
    {
        // just move shape up
        rect.setTop(rect.top() - 1);
        rect.setBottom(rect.bottom() - 1);

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

        rect = mChildShapes[0]->rect();
        rect.setTop(rect.top() - 1);
    }
    else if (mChildShapes.size() == 3)
    {
        int i = 0;
        int TODO5; // question logic
    }

    setRect(rect, false);
}

ValencyPoint ShapeItem::createValencyPoint(const QPointF& point, ValencyPoint::Role role)
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
    vPoint.setOwner(this);

    return vPoint;
}

void ShapeItem::createValencyPoints(Command* cmd)
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

    mValencyPoints.clear();
    qreal W = itemSize().width();
    qreal H = itemSize().height();
    qreal w = cellSize().width();
    qreal h = cellSize().height();

    DRAKON::IconType type = cmd->type();

    switch (type)
    {
    case DRAKON::BRANCH_BEGIN:
    case DRAKON::ACTION_MATH:
    case DRAKON::ACTION_MODULE:
    case DRAKON::DELAY:
        {
            ValencyPoint point = createValencyPoint(QPointF(W / 2, H - h / 2), ValencyPoint::Down);
            mValencyPoints.push_back(point);

            if (type == DRAKON::BRANCH_BEGIN && !isCyclogramEndBranch(cmd))
            {
                ValencyPoint point = createValencyPoint(QPointF(W, 0), ValencyPoint::Right);
                mValencyPoints.push_back(point);
            }
        }
        break;

    case DRAKON::QUESTION:
        {
            CmdQuestion* questionCmd = qobject_cast<CmdQuestion*>(cmd);
            if (questionCmd)
            {
                ValencyPoint rightPoint = createValencyPoint(QPointF(W, H / 2), ValencyPoint::Right);
                mValencyPoints.push_back(rightPoint);

                if (questionCmd->questionType() == CmdQuestion::IF)
                {
                    ValencyPoint downPoint = createValencyPoint(QPointF(W / 2, H - h / 2), ValencyPoint::Down);
                    mValencyPoints.push_back(downPoint);

                    ValencyPoint underArrowPoint = createValencyPoint(QPointF(W / 2, H + h / 2), ValencyPoint::UnderArrow);
                    mValencyPoints.push_back(underArrowPoint);

                }
                else if (questionCmd->questionType() == CmdQuestion::CYCLE)
                {
                    ValencyPoint downPoint = createValencyPoint(QPointF(W / 2, H), ValencyPoint::Down);
                    mValencyPoints.push_back(downPoint);

                    ValencyPoint underArrowPoint = createValencyPoint(QPointF(W / 2, h / 2), ValencyPoint::UnderArrow);
                    mValencyPoints.push_back(underArrowPoint);
                }
            }
        }
        break;
    default:
        break;
    }
}

bool ShapeItem::isCyclogramEndBranch(Command* cmd) const
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
        if (cmd->nextCommands()[i] && isCyclogramEndBranch(cmd->nextCommands()[i]))
        {
            return true;
        }
    }

    return false;
}
