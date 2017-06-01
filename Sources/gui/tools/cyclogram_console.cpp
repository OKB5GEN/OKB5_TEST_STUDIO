#include "Headers/gui/tools/cyclogram_console.h"
#include "Headers/logger/Logger.h"
#include "Headers/logic/command.h"

//#include "Headers/gui/tools/console_text_widget.h"

#include <QtWidgets>

CyclogramConsole::CyclogramConsole(QWidget * parent):
    QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);

    QPushButton* clearConsoleBtn = new QPushButton(QIcon(":/resources/images/delete_all"), tr("Clear"), this);

    QHBoxLayout* hLayout = new QHBoxLayout();
    layout->addLayout(hLayout, 0, 0);
    hLayout->addWidget(clearConsoleBtn);
    hLayout->addStretch();

    mTextEdit = new QTextEdit(this);
    mTextEdit->setReadOnly(true);

    QFont font;
    font.setPointSize(10);
    font.setFamily("Verdana");
    mTextEdit->setFont(font);

    layout->addWidget(mTextEdit, 1, 0);

    connect(clearConsoleBtn, SIGNAL(clicked()), mTextEdit, SLOT(clear()));
}

CyclogramConsole::~CyclogramConsole()
{

}

void CyclogramConsole::onCommandStarted(Command* command)
{
    if (command->onStartConsoleText().isEmpty())
    {
        return;
    }

    QString message = command->onStartConsoleText();
    //TODO parse text to find variable links/macroses etc

    addMessage(command->onStartConsoleTextColor(), message);

//    QStringList colors = ConsoleTextWidget::colorsList();

//    foreach (QString colorName, colors)
//    {
//        QColor color(colorName);
//        addMessage(color, colorName);
//    }
}

void CyclogramConsole::onCommandFinished(Command* command)
{
    if (command->onFinishConsoleText().isEmpty())
    {
        return;
    }

    QString message = command->onFinishConsoleText();
    //TODO parse text to find variable links/ьфскщыуы

    addMessage(command->onFinishConsoleTextColor(), message);
}

void CyclogramConsole::addMessage(const QColor& color, const QString& message)
{
    mTextEdit->setTextColor(color);

    QDateTime timeStamp = QDateTime::currentDateTimeUtc();
    QString msg = timeStamp.toString(QLatin1String("HH:mm:ss.zzz"));
    msg += ": ";
    msg += message;

    mTextEdit->append(msg);
}

void CyclogramConsole::clear()
{
    mTextEdit->clear();
}
