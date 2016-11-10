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


Cyclogram::Cyclogram(QObject * parent):
    QObject(parent)
{
}

void Cyclogram::createDefault()
{
     addCell(Cell(QPoint(0, 0), new CmdStateStart(START_STATE_NAME, this)));
     addCell(Cell(QPoint(0, 1), Cell::ADD_COMMAND));
     addCell(Cell(QPoint(0, 2), new CmdSetState(END_OK_STATE_NAME, this)));
     addCell(Cell(QPoint(1, 0), new CmdStateStart(END_OK_STATE_NAME, this)));
     addCell(Cell(QPoint(2, 0), new CmdStateStart(END_FAIL_STATE_NAME, this)));
}

void Cyclogram::addCell(const Cell& cell)
{

}
