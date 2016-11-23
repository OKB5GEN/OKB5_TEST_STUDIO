#ifndef CYCLOGRAM_H
#define CYCLOGRAM_H

#include <QObject>
#include <QList>
#include "Headers/cell.h"

class Command;

class Cyclogram: public QObject
{
    Q_OBJECT

public:
    const QString START_STATE_NAME = "START";
    const QString END_STATE_NAME = "END";

    Cyclogram(QObject * parent);

    void createDefault();
    void run();
    void stop();
    void pause();
    void resume();

    const QList<Cell>& cells() const;

private slots:
    void onCommandFinished(Command* cmd);

private:
    enum State // TODO correct cyclogram start/pause/stop -ing
    {
        STOPPED,
        RUNNING,
        PAUSED
    };

    void insertCell(const Cell& cell);

    QList<Cell> mCells;
    int mRows = 0;
    int mColumns = 0;
    Command* mFirst = Q_NULLPTR;
    Command* mCurrent = Q_NULLPTR;
    State mState = STOPPED;

signals:
    void changed();
};
#endif // CYCLOGRAM_H
