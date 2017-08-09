#include "Headers/logic/commands/cmd_parallel_process.h"
#include "Headers/logger/Logger.h"
#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

/* Brainstorm по реализации параллельного процесса

Итого основные моменты:

1. ПП - это по сути та же команда-"подпрограмма", только ее "запустили и забыли про нее" (в значке команды в GUI горит лампочка-индикатор)
2. При запуске ПП добавляется в список запущенных ПП вызвавшей циклограммы (для контроля)
3. В связке ПП + вызвавшая циклограмма (при редактировании команды) определяется "кто главный":
    - Если главная вызвавшая циклограмма, то при ее завершении она убивает ПП
    - Если главный ПП, то, когда вызывающая циклограмма дойдет до конца, она ждет пока ПП завершится
4. При запуске ПП возможны коллизии при работе с модулями (ПП1 говорит "выключи", ПП2 говорит "включи").
5. Коллизии есть "критичные" и "некритичные"
6. Критичные коллизии - это когда разные ПП шлют "управляющие" (меняющие состояние модуля) команды на модули (включить/выключить/установить)
7. Критичные коллизии отслеживаем на этапе загрузки циклограммы из файла, если встречаем вызовы в разных ПП.
8. Если критичные коллизии обнаружены, то такую циклограмму мы не даем запускать
9. Некритичные коллизии - это когда разные ПП шлют "опрашивающие" (не меняющие состояние модуля, а только получающие его текущее состояние/параметры)
10. Некритичные коллизии мы разрешаем через очередь доступа ПП к модулю (если модуль занят, то ПП встает в очередь, как модуль обработает текущий запрос, он оповестит ПП первый в очереди и тот начнет выполнение своей команды)
11. В ПП из вызывающей циклограммы можно передать переменные (как с подпрограммой). Эти переменные будут изменяться каждый раз, когда меняется значение переменной ПП, к которой она привязана (в подпрограмме они меняются только один раз, когда выполнение подпрограммы завершается)
12. В параметрах ПП также указывается стартуем ли мы ПП (указывается файл) или завершаем ПП (указывается имя). Нужно для ручной остановки ПП (если ПП нужен только на выполнение части циклограммы и смысла держать его до конца нет)
13. ПП различаются между собой по уникальному имени в пределах вызывающей циклограммы (как ветки циклограммы)
14. ПП стартует только, если он уже не запущен (если в цикле например пытаемся стартовать одну и ту же ПП команду)
15. Параллельный процесс не может сам себя вызывать (НЕТ РЕКУРСИИ АНАЛОГИЧНО ПОДПРОГРАММАМ!)

16. (Возможно оффтоп) Нужен аварийный механизм завершения циклограммы + возможно циклограммная команда "СТОП", которая по какой-то юзер-логике будет стопать всю циклограмму целиком
Hardcode + команда? По идее нужен механизм, независящий от пользователя
17. Скорее всего ПП должен быть "сам в себе", то есть мы его можем запустить с какими-то настройками, но получать из него мы ничего не можем?
Тупо по той причине, что данные могут быть не актуальные, либо нужны какие-то объекты синхронизации подобно потокам.
18. "Аварийное завершение" можно сделать так: завести в параллельном процессе код завершения типа ОК/ЖОПА. Если параллельный процесс обнаруживет жопу, то он выставляет код завершения и завершается.
Родительский процесс детектирует завершение своего параллельного процесса и смотрит код завершения. Если код ЖОПА, то схлопывает все (или пинает что-то наверх и через там все умирает)

19. Управляющие команды теоретически можно сделать из других процессов аналогично объектам синхронизации.
Например есть пара команд выключить/выключить реле нагревателя. Если какой-то процесс включил нагреватель, то его мы назначаем "ответственным" за освобождение этого ресурса
После того, как он выключил реле, мы можем его "занять". Но это будет пиздец :) Потому что хз какая логика будет навернута после того, как реле выключилось. Лучше не давать такой возмодности

20. Что делать при попытке многократно вызвать один и тот же параллельный процесс?
21. Как конфигурять параллельный процесс (завершение с ошибкой, нормальное завершение) чтобы это нормально обрабатывать в вызывающей циклограмме?
22. Как правильно килять параллельный процесс? Как конфигурять его на предмет того, что делать если родительский процесс тебя хочет стопнуть? убить? (например при закрытии приложения)
    (В дальнем квадратике ПУСК/ОСТАНОВ в ближнем квадратике наименование параллельного процесса - так вручную киляется, как обрабатывать самозавершение процесс - вопрос
    вероятно надо будет прокидывать ветку основного алгоритма в конфигурировании параллельного процесса типа "если процесс завершается, вызывается ветка такая-то" синхронно или асинхронно?)

*/

CmdParallelProcess::CmdParallelProcess(QObject* parent):
    Command(DRAKON::PARALLEL_PROCESS, 1, parent)
{
    mText = "Parallel Process";
}

void CmdParallelProcess::run()
{
    if (mExecutionDelay > 0)
    {
        QTimer::singleShot(mExecutionDelay, this, SLOT(finish()));
    }
    else
    {
        finish();
    }
}

void CmdParallelProcess::stop()
{
}

#ifdef ENABLE_CYCLOGRAM_PAUSE
void CmdParallelProcess::pause()
{
}

void CmdParallelProcess::resume()
{
}
#endif

void CmdParallelProcess::finish()
{
    emit finished(nextCommand());
}

void CmdParallelProcess::writeCustomAttributes(QXmlStreamWriter* writer)
{
//    writer->writeAttribute("delay", QString::number(mDelay));
}

void CmdParallelProcess::readCustomAttributes(QXmlStreamReader* reader, const Version& fileVersion)
{
//    QXmlStreamAttributes attributes = reader->attributes();
//    int delay = 0;
//    if (attributes.hasAttribute("delay"))
//    {
//        delay = attributes.value("delay").toInt();
//    }

//    setDelay(delay);
}

bool CmdParallelProcess::loadFromImpl(Command* other)
{
    CmdParallelProcess* otherParallelProcCmd = qobject_cast<CmdParallelProcess*>(other);
    if (!otherParallelProcCmd)
    {
        LOG_ERROR(QString("Command type mismatch (not parallel process)"));
        return false;
    }

    return true;
}
