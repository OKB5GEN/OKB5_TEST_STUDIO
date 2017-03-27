#include "Headers/gui/tools/app_console.h"
#include "Headers/logger/TextEditAppender.h"
#include "Headers/logger/Logger.h"

#include <QtWidgets>

AppConsole::AppConsole(QWidget * parent):
    QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QTextEdit* textEdit = new QTextEdit(this);
    //textEdit->setMinimumWidth(600);
    layout->addWidget(textEdit);

    mTextEditAppender = new TextEditAppender();
    mTextEditAppender->setTextEdit(textEdit);
    mTextEditAppender->setFormat("%{time}: %{message}");
    Logger::globalInstance()->registerAppender(mTextEditAppender);

    //setWindowTitle(tr("Application console"));
}

AppConsole::~AppConsole()
{
    //Logger::globalInstance()->removeAppender(mTextEditAppender);
}
