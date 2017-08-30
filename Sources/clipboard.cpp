#include "Headers/clipboard.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/command.h"
#include "Headers/logger/Logger.h"

Clipboard::Clipboard():
    QObject(Q_NULLPTR),
    mCommandToCopy(Q_NULLPTR)
{

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
    if (mCommandToCopy)
    {
        if (mCommandToCopy->type() != DRAKON::BRANCH_BEGIN)
        {
            mCyclogram->deleteCommand(mCommandToCopy);
        }
        else
        {
            mCyclogram->deleteBranch(mCommandToCopy);
        }
    }

    if (!Cyclogram::canBeCopied(command->type()))
    {
        mCommandToCopy = 0;
        mCyclogram.reset();
        return;
    }

    if (command->type() == DRAKON::BRANCH_BEGIN)
    {
        mCommandToCopy = createBranchCopy(command, cyclogram);
    }
    else
    {
        mCommandToCopy = createCommandCopy(command, cyclogram);
    }

    mCyclogram = cyclogram;
}

Command* Clipboard::createCommandCopy()
{
    return createCommandCopy(mCommandToCopy, mCyclogram);
}

Command* Clipboard::createBranchCopy()
{
    return createBranchCopy(mCommandToCopy, mCyclogram);
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
