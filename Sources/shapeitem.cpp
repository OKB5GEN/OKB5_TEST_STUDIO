#include <QtWidgets>

#include "Headers/shapeitem.h"
#include "Headers/command.h"

#include "Headers/commands/cmd_title.h"

namespace
{
    static const QSizeF CELL = QSizeF(30, 30);
    static const int CELLS_PER_ITEM_V = 4;
    static const int CELLS_PER_ITEM_H = 8;
}

QSizeF ShapeItem::smItemSize = QSizeF(30 * 8, 30 * 4);

ShapeItem::ShapeItem(QObject* parent):
    QObject(parent),
    mCommand(Q_NULLPTR)
{
    //smItemSize = QSizeF(CELL.width() * CELLS_PER_ITEM_H, CELL.height() * CELLS_PER_ITEM_V);

    mFont.setPointSize(14);
    mFont.setFamily("Arial");
}

QPainterPath ShapeItem::path() const
{
    return mPath;
}

QPainterPath ShapeItem::textPath() const
{
    return mTextPath;
}

QPoint ShapeItem::position() const
{
    return mPosition;
}

QColor ShapeItem::color() const
{
    return mColor;
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
    }

    mCommand = command;
    connect(mCommand, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));

    onTextChanged(mCommand->text());
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
}

QSizeF ShapeItem::itemSize()
{
    return smItemSize;
}

QSizeF ShapeItem::cellSize()
{
    return CELL;
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
    case DRAKON::ACTION:
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
            int TODO; // very complex logics for QUESTION connections drawing will be here

            path.moveTo(CELL.width(), smItemSize.height() / 2);
            path.lineTo(CELL.width() * 2, smItemSize.height() - CELL.height());
            path.lineTo(smItemSize.width() - 2 * CELL.width(), smItemSize.height() - CELL.height());
            path.lineTo(smItemSize.width() - CELL.width(), smItemSize.height() / 2);
            path.lineTo(smItemSize.width() - 2 * CELL.width(), CELL.height());
            path.lineTo(CELL.width() * 2, CELL.height());
            path.lineTo(CELL.width(), smItemSize.height() / 2);
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
    setColor(selected ? QColor::fromRgba(0xff00ff00) : QColor::fromRgba(0xffffffff));
}
