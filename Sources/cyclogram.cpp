#include "Headers/cyclogram.h"
#include "Headers/commands/cmd_state_start.h"
#include "Headers/commands/cmd_set_state.h"

// Описание архитектуры

/* Циклограмма строится на основе ДРАКОН-схемы и выполняется сверху вниз, переходя при смене состояния снова вверх
 * Представляет собой по сути STATE-машину и хранится в форме двуразмерного массива с (0;0) в левом верхнем углу.
 * 0-я строка содержит только команды "Начало состояния".
 * Последняя строка содержит только команды "Переход в состояние". Исключание составляют состояния "END_OK" и "END_FAIL".
 * В эти состояния нельзя добавлять операции - это команды успешного/неуспешного завершения всей циклограммы
 * Все команды создаются и выполняются между этими двумя строками.
 * В ячейке (0;0) по умолчанию всегда команда "Начало состояния" "START". Эта команда всегда выполятся самой первой.
 * В ячейке (0;1) по умолчанию всегда есть команда "Добавление команды" или "Плюсик"
 * В ячейке (0;2) по умолчанию всегда команда "Переход в состояние" "END_OK"
 * В ячейке (1;0) по умолчанию команда "Начало состояния" "END_OK". Эта команда всегда в предпоследнем столбце. Завершает выполнение циклограммы с рез-том "Успешно".
 * В ячейке (2;1) по умолчанию команда "Начало состояния" "END_FAIL". Эта команда всегда в последнем столбце. Завершает выполнение циклограммы  с рез-том "Неуспешно".
*/

/* Как это работает
 *
 * Есть редактор, он следит за расположением операций по ячейкам
 * Есть обработчик команд. Он знает только первую команду и запускает ее.
 * Команда по своем завешении присылает сигнал, в котором присылает указатель на следующую команду
 * Если команда присылает 0, то это значит окончание циклограммы
 * По итогу у каждой команды есть список команд, которые могут быть выполнены по ее завершении
 * Обратная связь команд я не уверен, что нужна
 *
 * Добавление/удаление команд
 *
 */

Cyclogram::Cyclogram(QObject * parent):
    QObject(parent)
{
}

void Cyclogram::createDefault()
{
    foreach (const Cell& cell, mCells)
    {
        Command* cmd = cell.command();
        if (cmd)
        {
            cmd->deleteLater();
        }
    }

    mCells.clear();

    CmdStateStart* first = new CmdStateStart(START_STATE_NAME, this);
    CmdStateStart* setEndState = new CmdStateStart(END_STATE_NAME, this);
    first->setNext(setEndState);
    mFirst = first;
    mCurrent = Q_NULLPTR;

    mCells.append(Cell(QPoint(mRows, 0), mFirst));
    mCells.append(Cell(QPoint(mRows, 1), setEndState));
    ++mRows;

    mCells.append(Cell(QPoint(mRows, 0), Cell::ADD_COMMAND));
    mCells.append(Cell(QPoint(mRows, 1), Cell::EMPTY));
    ++mRows;

    mCells.append(Cell(QPoint(mRows, 0), new CmdSetState(END_STATE_NAME, this)));
    mCells.append(Cell(QPoint(mRows, 1), Cell::EMPTY));
    ++mRows;
    mColumns = 2;

    emit changed();
}

void Cyclogram::insertCell(const Cell& cell)
{
    int row = cell.pos().x();
    int column = cell.pos().y();

    if (cell.command())
    {
        //QSize size = cell.getCommand()->getSize();
        //insertColumns(size.width() - 1);

        /* TODO НА ПОПОЗЖЕ, СНАЧАЛА БУДЕТ ТОЛЬКО ЛИНЕЙНОЕ ДОБАВЛЕНИЕ/УДАЛЕНИЕ КОМАНД
         *
         * Надо как-то правильно посчитать когда вставлять, а когда не вставлять строки и столбцы
         * Скорее всего как-то надо держать в уме:
         * - самую большую длину ветки
         * - длину текущей ветки
         * - ширину текущей ветки
         *
         * Алгоритм примерно такой (ппц):
         * - по координатам ячейки определяем в какую ветку мы будем вставляться
         * - видимо придется иметь QMap<QString, QPoint>, где в QPoint хрянится column min и column max,
         * из которых мы сможем определить в какую ветку вставляемся
         * - если "индекс колонки вставки + вставляемая ширина - 1 (наверное)" > "текущая ширина ветки",
         * то вставляем столбец, при этом все, что справа от вставляемого двигаем вправо
         * обновляем мапу новыми значениями
         * - далее смотрим в другой QMap<QString, int>, где указан row max для каждой ветки
         * - если мы вставляем в самую длинную ветку или не в самую длинную,
         * но "текущая длина + размер вставляемого - 1 (размер плюсика)" > "самой большой длины", то вставляем строки
         * при этом все, что ниже места строки вставки, двигается вниз
*/
    }

    /*
    // we can insert
    if ((row + 1) > mRows) // insert new row
    {
        ++mRows;

        if ((column + 1) > mColumns)
        {
            ++mColumns;
        }

        mCells.insert(column + row * mColumns, cell);
    }
    */
}

void Cyclogram::run()
{
    if (mState == STOPPED && mFirst != Q_NULLPTR)
    {
        mState = RUNNING;
        connect(mFirst, SIGNAL(onFinished(Command*)), this, SLOT(onCommandFinished(Command*)));
        mCurrent = mFirst;
        mCurrent->run();
    }
}

void Cyclogram::onCommandFinished(Command* cmd)
{
    if (cmd != Q_NULLPTR)
    {
        mCurrent = cmd;
        connect(mCurrent, SIGNAL(onFinished(Command*)), this, SLOT(onCommandFinished(Command*))); // TODO must be called on command creation

        if (mState == RUNNING)
        {
            mCurrent->run();
        }
    }
    else
    {
        stop();
    }
}

void Cyclogram::stop()
{
    if (mState == RUNNING || mState == PAUSED)
    {
        mCurrent->stop();
        mState = STOPPED;
        mCurrent = mFirst;
    }
}

void Cyclogram::pause()
{
    if (mState == RUNNING)
    {
        mCurrent->pause();
        mState = PAUSED;
    }
}

void Cyclogram::resume()
{
    if (mState == PAUSED)
    {
        mCurrent->resume();
        mState = RUNNING;
    }
}

const QList<Cell>& Cyclogram::cells() const
{
    return mCells;

}
