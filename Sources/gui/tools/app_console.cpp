#include "Headers/gui/tools/app_console.h"
#include "Headers/logger/TextEditAppender.h"
#include "Headers/logger/Logger.h"

#include <QtWidgets>

AppConsole::AppConsole(QWidget * parent):
    QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);

    QPushButton* clearConsoleBtn = new QPushButton(QIcon(":/images/delete_all"), tr("Clear console"), this);

    QHBoxLayout* hLayout = new QHBoxLayout();
    layout->addLayout(hLayout, 0, 0);
    hLayout->addWidget(clearConsoleBtn);
    hLayout->addStretch();

    QTextEdit* textEdit = new QTextEdit(this);
    layout->addWidget(textEdit, 1, 0);

    mTextEditAppender = new TextEditAppender();
    mTextEditAppender->setTextEdit(textEdit);
    mTextEditAppender->setFormat("%{time}: %{message}");
    Logger::globalInstance()->registerAppender(mTextEditAppender);

    connect(clearConsoleBtn, SIGNAL(clicked()), textEdit, SLOT(clear()));
}

AppConsole::~AppConsole()
{
    //Logger::globalInstance()->removeAppender(mTextEditAppender);
}
