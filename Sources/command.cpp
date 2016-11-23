#include "Headers/command.h"

Command::Command(ShapeTypes type, QObject * parent):
    QObject(parent),
    mNext(Q_NULLPTR),
    mType(type)
{

}

Command::~Command()
{

}

void Command::run()
{

}

void Command::stop()
{

}

void Command::pause()
{

}

void Command::resume()
{

}

void Command::setNext(Command* cmd)
{
    mNext = cmd;
}

ShapeTypes Command::type() const
{
    return mType;
}
