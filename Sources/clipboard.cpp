#include "Headers/clipboard.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/command.h"
#include "Headers/logger/Logger.h"

Clipboard::Clipboard():
    QObject(Q_NULLPTR),
    mCommandToCopy(Q_NULLPTR)
{
    mCyclogram.reset(new Cyclogram(Q_NULLPTR));
}

Clipboard::~Clipboard()
{

}

Command* Clipboard::commandToCopy() const
{
    return mCommandToCopy;
}

void Clipboard::setCommandToCopy(Command* command, QSharedPointer<Cyclogram> cyclogram)
{
    mCyclogram->clear();
    mCommandToCopy = 0;

    if (!Cyclogram::canBeCopied(command->type()))
    {
        return;
    }

    if (command->type() == DRAKON::BRANCH_BEGIN)
    {
        mCommandToCopy = createBranchCopy(command, mCyclogram);
    }
    else
    {
        mCommandToCopy = createCommandCopy(command, mCyclogram);
    }
}

Command* Clipboard::createCommandCopy(QSharedPointer<Cyclogram> cyclogram)
{
    return createCommandCopy(mCommandToCopy, cyclogram);
}

Command* Clipboard::createBranchCopy(QSharedPointer<Cyclogram> cyclogram)
{
    return createBranchCopy(mCommandToCopy, cyclogram);
}

Command* Clipboard::createCommandCopy(Command* from, QSharedPointer<Cyclogram> cyclogram)
{
    Command* newCmd = Q_NULLPTR;

    if (!from || !cyclogram)
    {
        LOG_ERROR(QString("Ivalid command copy data"));
        return newCmd;
    }

    newCmd = cyclogram->createCommand(from->type());
    newCmd->copyFrom(from);
    return newCmd;
}

Command* Clipboard::createBranchCopy(Command* from, QSharedPointer<Cyclogram> cyclogram)
{
    Command* newCmd = Q_NULLPTR;

    if (!from || !cyclogram || from->type() != DRAKON::BRANCH_BEGIN)
    {
        LOG_ERROR(QString("Ivalid branch copy data"));
        return newCmd;
    }

    newCmd = cyclogram->createBranchCopy(from);
    return newCmd;
}

void Clipboard::setSystemState(SystemState* state)
{
    mSystemState = state;
    mCyclogram->setSystemState(mSystemState);
}

QSharedPointer<Cyclogram> Clipboard::cyclogram() const
{
    return mCyclogram;
}
