#include <QtWidgets>

#include "Headers/gui/cyclogram/shape_item.h"
#include "Headers/logic/command.h"

#include "Headers/logic/commands/cmd_title.h"
#include "Headers/logic/commands/cmd_question.h"

namespace
{
    static const QSizeF CELL = QSizeF(30, 30);
    static const int CELLS_PER_ITEM_V = 4;
    static const int CELLS_PER_ITEM_H = 8;
}

QSizeF ShapeItem::smItemSize = QSizeF(30 * 8, 30 * 4); // TODO make function for initialization

ShapeItem::ShapeItem(QObject* parent):
    QObject(parent),
    mCommand(Q_NULLPTR),
    mActive(false)
{
    //smItemSize = QSizeF(CELL.width() * CELLS_PER_ITEM_H, CELL.height() * CELLS_PER_ITEM_V);

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
    qreal x = (smItemSize.width() - textRect.width()) / 2;
    qreal y = (smItemSize.height() + textRect.height()) / 2;
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

            qreal x1 = smItemSize.width() - CELL.width();
            qreal y1 = smItemSize.height() / 2 - CELL.width() * 0.1;
            additionalText.addText(x1, y1, font, yesDown ? tr("No") : tr("Yes"));

            qreal x2 = smItemSize.width() / 2 + CELL.width() / 3;
            qreal y2 = smItemSize.height() - CELL.height() * 0.6;
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

QSizeF ShapeItem::itemSize()
{
    return smItemSize;
}

QSizeF ShapeItem::cellSize()
{
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

    switch (type)
    {
    case DRAKON::TERMINATOR:
        {
            qreal yOffset = (cell.y() - itemRect.top()) * smItemSize.height();
            qreal radius = (smItemSize.height() - 2 * CELL.height()) / 2;
            QRectF rect(CELL.width(), CELL.height(), smItemSize.width() - 2 * CELL.width(), 2 * radius);
            path.addRoundedRect(rect, radius, radius);

            // connector
            CmdTitle* titleCmd = qobject_cast<CmdTitle*>(command());
            if (titleCmd)
            {
                if (titleCmd->titleType() == CmdTitle::BEGIN)
                {
                    path.moveTo(smItemSize.width() / 2, smItemSize.height() - CELL.height());
                    path.lineTo(smItemSize.width() / 2, smItemSize.height());
                }
                else // END terminator
                {
                    path.moveTo(smItemSize.width() / 2, CELL.height());
                    path.lineTo(smItemSize.width() / 2, -yOffset);
                }
            }
        }
        break;
    case DRAKON::BRANCH_BEGIN:
        {
            path.moveTo(CELL.width(), CELL.height());
            path.lineTo(CELL.width(), smItemSize.height() - CELL.height() * 3 / 2);
            path.lineTo(smItemSize.width() / 2, smItemSize.height() - CELL.height());
            path.lineTo(smItemSize.width() - CELL.width(), smItemSize.height() - CELL.height() * 3 / 2);
            path.lineTo(smItemSize.width() - CELL.width(), CELL.height());
            path.lineTo(CELL.width(), CELL.height());
        }
        break;
    case DRAKON::GO_TO_BRANCH:
        {
            path.moveTo(CELL.width(), CELL.height() * 3 / 2);
            path.lineTo(CELL.width(), smItemSize.height() - CELL.height());
            path.lineTo(smItemSize.width() - CELL.width(), smItemSize.height() - CELL.height());
            path.lineTo(smItemSize.width() - CELL.width(), CELL.height() * 3 / 2);
            path.lineTo(smItemSize.width() / 2, CELL.height());
            path.lineTo(CELL.width(), CELL.height() * 3 / 2);
        }
        break;
    case DRAKON::ACTION_MATH:
    case DRAKON::ACTION_MODULE:
        {
            path.addRect(QRect(CELL.width(), CELL.height(), smItemSize.width() - 2 * CELL.width(), smItemSize.height() - 2 * CELL.height()));
        }
        break;
    case DRAKON::DELAY:
        {
            path.moveTo(CELL.width(), CELL.height());
            path.lineTo(CELL.width() * 2, smItemSize.height() - CELL.height());
            path.lineTo(smItemSize.width() - 2 * CELL.width(), smItemSize.height() - CELL.height());
            path.lineTo(smItemSize.width() - CELL.width(), CELL.height());
            path.lineTo(CELL.width(), CELL.height());
        }
        break;

    case DRAKON::QUESTION:
        {
            CmdQuestion * questionCmd = qobject_cast<CmdQuestion*>(command());

            if (questionCmd)
            {
                int TODO; // very complex logics for QUESTION connections drawing will be here

                path.moveTo(CELL.width(), smItemSize.height() / 2);
                path.lineTo(CELL.width() * 2, smItemSize.height() - CELL.height());
                path.lineTo(smItemSize.width() - 2 * CELL.width(), smItemSize.height() - CELL.height());
                path.lineTo(smItemSize.width() - CELL.width(), smItemSize.height() / 2);
                path.lineTo(smItemSize.width() - 2 * CELL.width(), CELL.height());
                path.lineTo(CELL.width() * 2, CELL.height());
                path.lineTo(CELL.width(), smItemSize.height() / 2);

                QPainterPath addPath;
                if (questionCmd->questionType() == CmdQuestion::IF)
                {
                    addPath.moveTo(smItemSize.width() - CELL.width(), smItemSize.height() / 2);
                    addPath.lineTo(smItemSize.width(), smItemSize.height() / 2);
                    addPath.lineTo(smItemSize.width(), smItemSize.height());
                    addPath.lineTo(smItemSize.width() / 2, smItemSize.height());
                }
                else if (questionCmd->questionType() == CmdQuestion::CYCLE)
                {
                    addPath.moveTo(smItemSize.width() - CELL.width(), smItemSize.height() / 2);
                    addPath.lineTo(smItemSize.width(), smItemSize.height() / 2);
                    addPath.lineTo(smItemSize.width(), 0);
                    addPath.lineTo(smItemSize.width() / 2, 0);

                    QPainterPath arrowPath;
                    QPoint pos(smItemSize.width() / 2, 0);
                    arrowPath.moveTo(pos);
                    arrowPath.lineTo(QPoint(pos.x() + ShapeItem::cellSize().width(), pos.y() + ShapeItem::cellSize().height() / 4));
                    arrowPath.lineTo(QPoint(pos.x() + ShapeItem::cellSize().width(), pos.y() - ShapeItem::cellSize().height() / 4));
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
        qreal yOffset = (cell.y() - itemRect.top()) * smItemSize.height();

        // lower connector
        path.moveTo(smItemSize.width() / 2, smItemSize.height() - CELL.height());
        path.lineTo(smItemSize.width() / 2, smItemSize.height());
        // upper connector
        path.moveTo(smItemSize.width() / 2, CELL.height());
        path.lineTo(smItemSize.width() / 2, -yOffset);
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
