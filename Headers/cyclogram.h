#ifndef CYCLOGRAM_H
#define CYCLOGRAM_H

#include <QObject>
//#include <QList>
#include "Headers/cell.h"

class Command;

class Cyclogram: public QObject
{
    Q_OBJECT

public:
    const QString START_STATE_NAME = "START";
    const QString END_OK_STATE_NAME = "END_OK";
    const QString END_FAIL_STATE_NAME = "END_FAIL";

    const int MAX_COLUMNS = 10;
    const int MAX_ROWS = 10;

    Cyclogram(QObject * parent);

    void createDefault();


private:
    void addCell(const Cell& cell);

    //Cell mCells[MAX_ROWS][MAX_COLUMNS]; // memory overhead but modification will be quick

    //int mRows = 0;
    //int mColumns = 0;

signals:
};
#endif // CYCLOGRAM_H
