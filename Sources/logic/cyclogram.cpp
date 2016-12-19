#include <QTime>
#include <QTimer>
#include <QMetaEnum>

#include "Headers/logic/cyclogram.h"
#include "Headers/logic/commands/cmd_state_start.h"
#include "Headers/logic/commands/cmd_set_state.h"
#include "Headers/logic/commands/cmd_title.h"
#include "Headers/logic/commands/cmd_delay.h"
#include "Headers/logic/commands/cmd_action.h"
#include "Headers/logic/commands/cmd_action_math.h"
#include "Headers/logic/commands/cmd_question.h"

#include "Headers/logic/variable_controller.h"

/* Переменные циклограммы (мысли вслух)
 * 1. Каждая циклограмма может иметь свои переменные
 * 2. Переменные видны только в пределах своей циклограммы (за пределамы не видны)
 * 3. Любая команда может использовать любую переменную своей циклограммы
 * 4. ЗНАЧЕНИЯ переменных могут передаваться между циклограммами
 * 5. Циклограмма может имееть входные и выходные переменные
 * 6. Переменные принимаются извне и передаются наружу в том порядке, в котором они объявлены в своей циклограмме
 * 7. Если значение извне не передано, то оно инициализируется по умолчанию нулем
 * 8. Все переменные - вещественные
 * 9. Наверное все же стоит разделять переменные
 * 10. Переменные объявленные у стартового ТЕРМИНАТОРА - входные переменные
 * 11. Переменные объявленные у конечного ТЕРМИНАТОРА - выходные переменные
 * 12. Передача-возврат переменных осуществляется с помощью опциональной команды ПОЛКА, которая будет крепиться к квадратику ПОДПРОГРАММА
 * 13. Вверху ПОЛКИ будут входные параметры циклограммы (указываем их как ВХ12345..)
 * 14. Внизу ПОЛКИ будут возвращаемые параметры (ВЫХ1 = I, ВЫХ2=J)
 * 15. Передаваемых параметров в подпрограмму должно быть НЕ МЕНЬШЕ, чем входных параметров у нее
 *
 * 16. (ПЕРСПЕКТИВА) Приниматься и возвращаться могут также массивы (БОЛЬШАЯ ТЕМА НА ПЕРСПЕКТИВУ)
 * 17. (ПЕРСПЕКТИВА) Если после имени переменных идут [], то эта переменная - массив
 *
 * ПЕРВОНАЧАЛЬНЫЙ ВАРИАНТ:
 * 1. У ТЕРМИНАТОРА "СТАРТ" добавляем квадратик "ПАРАМЕТРЫ" для переменных
 * 2. В нем объявляем параметры циклограммы
 * 3. Переменные в циклограмме используем
 *
*/

namespace
{
    static const int COMMAND_RUN_INTERVAL = 10; // msec

    void LogCmd(Command* cmd, const QString& state)
    {
        static QMetaEnum metaEnum;
        if (metaEnum.keyCount() == 0)
        {
            metaEnum = QMetaEnum::fromType<DRAKON::IconType>();
        }

        QString text(metaEnum.valueToKey(cmd->type()));
        text += ":";
        text += cmd->text();

        qDebug("[%s] Command %s %s", qUtf8Printable(QTime::currentTime().toString()), qUtf8Printable(text), qUtf8Printable(state));
    }
}

Cyclogram::Cyclogram(QObject * parent):
    QObject(parent),
    mState(STOPPED)
//  , mExecuteOneCmd(false)
{
    mVarController = new VariableController(this);
}

void Cyclogram::createDefault()
{
    clear();

    // empty silhouette
    Command* begin = createCommand(DRAKON::TERMINATOR);
    {
        CmdTitle* tmp = qobject_cast<CmdTitle*>(begin);
        tmp->setTitleType(CmdTitle::BEGIN);
    }

    Command* branch1 = createCommand(DRAKON::BRANCH_BEGIN);
    {
        CmdStateStart* tmp = qobject_cast<CmdStateStart*>(branch1);
        tmp->setText(tr("Start"));
    }

    Command* toBranch2 = createCommand(DRAKON::GO_TO_BRANCH);
    {

    }

    Command* branch2 = createCommand(DRAKON::BRANCH_BEGIN);
    {
        CmdStateStart* tmp = qobject_cast<CmdStateStart*>(branch2);
        tmp->setText(tr("End"));
    }

    Command* end = createCommand(DRAKON::TERMINATOR);
    {
        CmdTitle* tmp = qobject_cast<CmdTitle*>(end);
        tmp->setTitleType(CmdTitle::END);
    }

    begin->addCommand(branch1);
    branch1->addCommand(toBranch2);
    branch1->setChildCommand(toBranch2, 0);
    toBranch2->addCommand(branch2);
    branch2->addCommand(end);
    branch2->setChildCommand(end, 0);

    mFirst = begin;
    mLast = end;

    mCurrent = Q_NULLPTR;

    emit changed();
}

void Cyclogram::run()
{
    qDebug("==================================");
    qDebug("[%s] Cyclogram started", qUtf8Printable(QTime::currentTime().toString()));
    qDebug("==================================");

    if (mState == STOPPED && mFirst != Q_NULLPTR)
    {
        mVarController->restart();
        mCurrent = mFirst;
        setState(RUNNING);
        runCurrentCommand();
    }
}

void Cyclogram::onCommandFinished(Command* cmd)
{
    mCurrent->setActive(false);
    disconnect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
    disconnect(mCurrent, SIGNAL(criticalError(Command*)), this, SLOT(onCriticalError(Command*)));
    LogCmd(mCurrent, "finished");

    if (cmd != Q_NULLPTR)
    {
        mCurrent = cmd;

        if (mState == RUNNING)
        {
            QTimer::singleShot(COMMAND_RUN_INTERVAL, this, SLOT(runCurrentCommand())); // to avoid direct slot after signal calling run next command after short timeout
        }
    }
    else
    {
        qDebug("==================================");
        qDebug("[%s] Cyclogram finished", qUtf8Printable(QTime::currentTime().toString()));
        qDebug("==================================");
        stop();
        emit finished("");
    }
}

void Cyclogram::onCriticalError(Command* cmd)
{
    disconnect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
    disconnect(mCurrent, SIGNAL(criticalError(Command*)), this, SLOT(onCriticalError(Command*)));
    LogCmd(mCurrent, "critical error: " + cmd->errorDesc());

    qDebug("==================================");
    qDebug("[%s] Cyclogram stopped due to critical runtime error", qUtf8Printable(QTime::currentTime().toString()));
    qDebug("==================================");

    stop();
    emit finished(tr("Critical error occured: ") + cmd->errorDesc());
}

void Cyclogram::runCurrentCommand()
{
    if (mCurrent)
    {
        connect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
        connect(mCurrent, SIGNAL(criticalError(Command*)), this, SLOT(onCriticalError(Command*)));

//        if (mExecuteOneCmd && QObject::sender())
//        {
//            setState(PAUSED);
//        }
//        else
//        {
            LogCmd(mCurrent, "started");
            mCurrent->setActive(true);
            mCurrent->run();
//        }
    }
}

void Cyclogram::stop()
{
    if (mState == RUNNING || mState == PAUSED)
    {
        mCurrent->setActive(false);
        disconnect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
        disconnect(mCurrent, SIGNAL(criticalError(Command*)), this, SLOT(onCriticalError(Command*)));
        mCurrent->stop();
    }

    mCurrent = mFirst;
    setState(STOPPED);
}

void Cyclogram::pause()
{
    if (mState == RUNNING)
    {
        mCurrent->pause();
        setState(PAUSED);
    }
}

void Cyclogram::resume()
{
    if (mState == PAUSED)
    {
        mCurrent->resume();
        setState(RUNNING);
    }
}
/*
void Cyclogram::setExecuteOneCmd(bool enable)
{
    mExecuteOneCmd = enable;
}
*/

Command* Cyclogram::first() const
{
    return mFirst;
}

Command* Cyclogram::last() const
{
    return mLast;
}

Command* Cyclogram::current() const
{
    return mCurrent;
}

Cyclogram::State Cyclogram::state() const
{
    return mState;
}

void Cyclogram::clear()
{
    if (mFirst)
    {
        deleteCommandTree(mFirst);
    }

    mFirst = Q_NULLPTR;
    mCurrent = Q_NULLPTR;
    mLast = Q_NULLPTR;
    mCommands.clear();
}

void Cyclogram::deleteCommandTree(Command* cmd)
{
    emit deleted(cmd);

    for (int i = 0, sz = mCommands.size(); i < sz; ++i)
    {
        if (mCommands[i] == cmd)
        {
            mCommands.takeAt(i);
            break;
        }
    }

    for (int i = 0, sz = cmd->nextCommands().size(); i < sz; ++i)
    {
        deleteCommandTree(cmd->nextCommands()[i]);
    }

    cmd->deleteLater();
}

void Cyclogram::deleteCommand(Command* cmd, bool recursive /*= false*/)
{
    if (recursive)
    {
        deleteCommandTree(cmd);
        return;
    }

    emit deleted(cmd);

    int TODO; // this is valid for one-column branches only!
    Command* parentCmd = cmd->parentCommand();
    Command* nextCmd = cmd->nextCommands()[0]; // TODO QUESTION deletion
    parentCmd->replaceCommand(nextCmd, nextCmd->role());

    for (int i = 0, sz = mCommands.size(); i < sz; ++i)
    {
        if (mCommands[i] == cmd)
        {
            Command* tmp = mCommands.takeAt(i);
            tmp->deleteLater();
            break;
        }
    }
}

Command* Cyclogram::createCommand(DRAKON::IconType type, int param /*= -1*/)
{
    Command* cmd = Q_NULLPTR;

    switch (type)
    {
    case DRAKON::TERMINATOR:
        {
            cmd = new CmdTitle(this);
        }
        break;
    case DRAKON::BRANCH_BEGIN:
        {
            cmd = new CmdStateStart(this);
        }
        break;
    case DRAKON::GO_TO_BRANCH:
        {
            cmd = new CmdSetState(this);
        }
        break;
    case DRAKON::DELAY:
        {
            cmd = new CmdDelay(this);
        }
        break;
    case DRAKON::ACTION_MATH:
        {
            CmdActionMath* tmp = new CmdActionMath(this);
            tmp->setVariableController(mVarController);
            cmd = tmp;
        }
        break;

    case DRAKON::QUESTION:
        {
            CmdQuestion* tmp = new CmdQuestion(this);
            tmp->setVariableController(mVarController);

            if (param == CmdQuestion::IF)
            {
                tmp->setQuestionType(CmdQuestion::IF);
            }
            else if (param == CmdQuestion::CYCLE)
            {
                tmp->setQuestionType(CmdQuestion::CYCLE);
            }

            cmd = tmp;
        }
        break;
    case DRAKON::ACTION_MODULE:
        {
            //cmd = new CmdActionModule(this);
        }
        break;

        //TODO not implemented
    case DRAKON::SWITCH:{} break;
    case DRAKON::CASE:{} break;
    case DRAKON::SUBPROGRAM:{} break;
    case DRAKON::SHELF:{} break;
    case DRAKON::FOR_BEGIN:{} break;
    case DRAKON::FOR_END:{} break;
    case DRAKON::OUTPUT:{} break;
    case DRAKON::INPUT:{} break;
    case DRAKON::START_TIMER:{} break;
    case DRAKON::SYNCHRONIZER:{} break;
    case DRAKON::PARALLEL_PROCESS:{} break;
    default:
        break;
    }

    if (cmd)
    {
        mCommands.push_back(cmd);
    }

    return cmd;
}

const QList<Command*>& Cyclogram::commands() const
{
    return mCommands;
}

void Cyclogram::setState(State state)
{
    static QMetaEnum metaEnum;
    if (metaEnum.keyCount() == 0)
    {
        metaEnum = QMetaEnum::fromType<Cyclogram::State>();
    }

    QString text(metaEnum.valueToKey(state));

    qDebug("[%s] Cyclogram state changed to %s", qUtf8Printable(QTime::currentTime().toString()), qUtf8Printable(text));

    mState = state;
    emit stateChanged(mState);
}

Command* Cyclogram::validate() const
{
    foreach (Command* command, mCommands)
    {
        if (command->hasError())
        {
            return command;
        }
    }

    return Q_NULLPTR;
}

VariableController* Cyclogram::varCtrl() const
{
    return mVarController;
}

void Cyclogram::getBranches(QList<Command*>& branches) const
{
    if (!mFirst || !mLast)
    {
        return;
    }

    // find first and last branches
    Command* firstBranch = mFirst->nextCommands()[0];
    Command* lastBranch = mLast->parentCommand(); //TODO parent command
    while (lastBranch->type() != DRAKON::BRANCH_BEGIN)
    {
        lastBranch = lastBranch->parentCommand();
    }

    // get other branches and draw them
    foreach (Command* it, mCommands)
    {
        if (it->type() == DRAKON::BRANCH_BEGIN && it != firstBranch && it != lastBranch)
        {
            branches.push_back(it);
        }
    }

    branches.push_front(firstBranch);
    branches.push_back(lastBranch);
}