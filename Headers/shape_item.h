#ifndef SHAPE_ITEM_H
#define SHAPE_ITEM_H

#include <QColor>
#include <QPainterPath>
#include <QPoint>
#include <QFont>

#include "Headers/valency_point.h"

class Command;

class ShapeItem: public QObject
{
    Q_OBJECT

public:
    ShapeItem(QObject* parent);

    void createPath();

    void setPath(const QPainterPath &path);
    void setTextPath(const QPainterPath &path);
    void setToolTip(const QString &toolTip);
    void setPosition(const QPoint &position);
    void setColor(const QColor &color);
    void setCell(const QPoint &position);
    void setCommand(Command* command);
    void setValencyPoints(const QList<ValencyPoint>& points);
    void setRect(const QRect& rect);
    void setSelected(bool selected);

    QPainterPath path() const;
    QPainterPath textPath() const;
    QPoint position() const;
    QColor color() const;
    QString toolTip() const;
    QPoint cell() const;
    Command* command() const;
    const QList<ValencyPoint>& valencyPoints() const;
    ValencyPoint valencyPoint(int role) const;
    QRect rect() const;

    static QSizeF itemSize();
    static QSizeF cellSize();

signals:
    void changed();

private slots:
    void onTextChanged(const QString& text);
    void onErrorStatusChanged(bool status);

private:
    QPainterPath mPath; // shape path
    QPainterPath mTextPath; // path for text iside shape
    QPoint mPosition; // top-left corner of the shape in window coordinates
    QPoint mCell; // cell, occupied by command shape itself
    QRect mRect; // bounding rect of the command (need for drawing command connections and QUESTION arrow)
    QColor mColor; // color for filling shape background
    QString mToolTip; // tooltip text for shape
    QList<ValencyPoint> mValencyPoints; // valency points list for this command

    QFont mFont; // font for writing texts

    static QSizeF smItemSize;

    Command* mCommand = Q_NULLPTR; // data pointer for the command logics
};

#endif //SHAPE_ITEM_H
