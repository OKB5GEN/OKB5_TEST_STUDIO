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
        int index = commandAt(helpEvent->pos());
        if (index != -1)
        {
            QToolTip::showText(helpEvent->globalPos(), mCommands[index].toolTip());
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

void SortingBox::drawItems(QList<ShapeItem>& items, QPainter& painter)
{
    // draw commands shapes first
    foreach (ShapeItem shapeItem, items)
    {
        painter.translate(shapeItem.position());
        painter.setBrush(shapeItem.color());
        painter.drawPath(shapeItem.path());
        painter.drawPath(shapeItem.textPath());
        painter.translate(-shapeItem.position());
    }

    // draw valency point secont (to be above the commands shapes)
    foreach (ShapeItem shapeItem, items)
    {
        painter.translate(shapeItem.position());

        foreach (ValencyPoint point, shapeItem.valencyPoints())
        {
            painter.setBrush(point.color());
            painter.drawPath(point.path());
        }

        painter.translate(-shapeItem.position());
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

int SortingBox::commandAt(const QPoint &pos)
{
    for (int i = mCommands.size() - 1; i >= 0; --i)
    {
        const ShapeItem &item = mCommands[i];
        if (item.path().contains(pos - item.position()))
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
        const QList<ValencyPoint>& points = mCommands[i].valencyPoints();
        for (int j = 0, sz = points.size(); j < sz; ++j)
        {
            if (points[j].path().contains(pos - mCommands[i].position()))
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

void SortingBox::createCommandShape(Command* cmd, const QPoint& cell)
{
    QPoint pos(mOrigin.x() + cell.x() * mItem.width(), mOrigin.y() + cell.y() * mItem.height());

    int TODO; //неправильная логика расширения размеров циклограммы при вставке в середину, тут только случай вставки в конец!!!

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
    shapeItem.setValencyPoints(createValencyPoints(cmd));
    addText(shapeItem);
    mCommands.append(shapeItem);

    update();
}

void SortingBox::addText(ShapeItem& item)
{
    int TODO; // move it to the shape item

    QPainterPath textPath;
    QFontMetrics fm(mFont);
    QRect textRect = fm.boundingRect(item.command()->text());
    qreal x = (mItem.width() - textRect.width()) / 2;
    qreal y = (mItem.height() + textRect.height()) / 2;
    textPath.addText(x, y, mFont, item.command()->text());
    item.setTextPath(textPath);
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
    foreach (ShapeItem shape, mCommands)
    {
        Command* command = shape.command();
        if (command->type() == ShapeTypes::BRANCH_BEGIN && command->text() == goToBranchCmd->text())
        {
            return true;
        }
    }

    return false;
}

QPainterPath SortingBox::createPath(Command* cmd)
{
    ShapeTypes type = cmd->type();
    QPainterPath path;

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
    /* Как добавлять команду в циклограмму (мысли вслух)?
     *
     * 1. Команда добавляется в точку валентности
     * 2. Точка валентности принадлежит команде
     * 3. Поэтому снчала надо создать команду в логике, а потом правильно ее отрисовать
     * 4. Создание команды в логике - это:
     *    - создать новый объект
     *    - в качестве следующей команды впихнуть в него next-команду, привязанную к точке валентности
     *    - команде-владельцу точки валентности в качестве next прописать новую команду
     * 5. После этого мы имеем команду в логике и теперь ее надо правильно отобразить в циклограмме
     * 6. Предположительно в параметрах команды должен появиться параметр "Прямоугольник в графике", размером от top-left ячейки до bottom-right
     * 7. Этот прямоугольник задает область расположения команды
     * 8. Сама команда всегда расположена в левом нижнем углу своего прямоугольника (допустим)
     * 9. От нее до top-left ячейки веритикально рисуется линия (сейчас пока просто полклетки по прямой рисуется).
     * 10. При этом cell и position (надо избавиться от двух сущностей - либо в ячейках считать либо в позициях) двигается на 1 строчку вниз
     * 11. При добавлении новой команды также нужно пробежаться по всем остальным командам
     * 12. Все команды, которые расположены на одной высоте по Y сдвигаются на однй строчку вниз, при этом их top-left остается прежним
     *
     * При отрисовке QUESTION будут свои свистопляски, но пока для упрощения забьем и сделаем линейно-бранчевые циклограммы
     *
     * Если коротко, то с QUESION (как и SWITCH-CASE) надо будет как-то мониторить стрелку ветвления и делать хитрожопую логику ее отрисовки
     * Логика точек валентности при этом будет завязана на то является ли ее хозяином QUESION, кто яаляется следующей командой
     * Наприммер, если хозяин - это IF-QUESTION, а след команда - GO_TO_BRANCH, то сюда можно вставить GO_TO_BRANCH и при вставке разорвать петлю IF'а
     *
     * Наиболее вероятно то, что QUESTION потребует хранения какой-то информации о дереве подобъектов
     * Rect этого QUESTION'а будет bounding rect'ом этого дерева команд
     * При добавлении команды нужно проверять не состоит ли команда в дереве какого-то QUESTIONа
     * Если состояит, то надо обновить rect'ы всех QUESTION'ов куда входит команда
     *
     * В итоге рисовать коннектторя надо по следующему алгоритму:
     * Есть Rect команды
     * - от ячейки, где находится position команды рисуем вверх линию от (position + CELL.height()) до top rect'а
     * - при этом по пути до top rectа проверяем нет ли в ячейке команды (если есть, то рисование прекращаем) - это для QUESTION'а типа CYCLE
     * - от ячейки position + mItem.height - CELL.height рисуем линию вниз до bottom rect'а
     * - опять же если по пути встречаем команду в ячейке, то рисование прекращаем - опять актуально для QUESTION'а
     * - QUESTION по Х всегда находтся в левом столбце своего rectа, по Y же может плавать в зависимости от наполнения веток дерева
     * - Дополнительно для QUESTION'а нужно отрисовывать стрелку
     *
     *
    */

}
































