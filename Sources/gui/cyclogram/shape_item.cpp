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
    mActive(false)
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

void ShapeItem::setPosition(const QPoint &position)
{
    mPosition = position;
}

void ShapeItem::setColor(const QColor &color)
{
    mColor = color;
}

QPoint ShapeItem::cell() const
{
    return mCell;
}

void ShapeItem::setCell(const QPoint &position)
{
    mCell = position;
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
    ValencyPoint point;
    for (int i = 0, sz = mValencyPoints.size(); i < sz; ++i)
    {
        if (role == mValencyPoints[i].role())
        {
            return mValencyPoints[i];
        }
    }

    int TODO; // make point not found notification
    return point;
}

void ShapeItem::setRect(const QRect& rect)
{
    mRect = rect;
}

QRect ShapeItem::rect() const
{
    return mRect;
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

void ShapeItem::setActive(bool active)
{
    mActive = active;
    emit changed();
}

void ShapeItem::createPath()
{
    DRAKON::IconType type = command()->type();
    QPainterPath path;

    QRect itemRect = rect();
    QPoint cell = mCell;

    qreal W = itemSize().width();
    qreal H = itemSize().height();
    qreal w = cellSize().width();
    qreal h = cellSize().height();

    switch (type)
    {
    case DRAKON::TERMINATOR:
        {
            qreal yOffset = (cell.y() - itemRect.top()) * H;
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
        qreal yOffset = (cell.y() - itemRect.top()) * H;

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
