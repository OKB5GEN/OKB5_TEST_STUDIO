#ifndef CELL_H
#define CELL_H

#include <QPoint>
#include <QString>

class Command;

class Cell
{
public:
    enum Type
    {
        EMPTY,      // no any operation performed, only connector will be drawn
        ADD_COMMAND,// command can be inserted in this cell
        COMMAND     // command must be executed in this cell
    };

    Cell();
    Cell(QPoint pos, Type type);
    Cell(QPoint pos, Command* cmd);

    const QPoint& pos() const;
    void setPos(const QPoint& pos);

    Type type() const;
    void setType(Type type);

    Command* command() const;
    void setCommand(Command* command);

    const QString& text() const;
    void setText(const QString& text);

private:
    QPoint mPos;
    Type mType = EMPTY;
    Command * mCommand = Q_NULLPTR;
    QString mText;
};
#endif // CELL_H
