#include "Headers/gui/tools/cyclogram_console.h"
#include "Headers/logger/Logger.h"
#include "Headers/logic/command.h"
#include "Headers/logic/variable_controller.h"

#include <QtWidgets>

namespace
{
    static const QString DELIMITER = "%";
}

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
}

void CyclogramConsole::onCommandFinished(Command* command)
{
    if (command->consoleText().isEmpty())
    {
        return;
    }

    // replace macroses to variables values
    QString message = command->consoleText();
    QStringList tokens = message.split(DELIMITER);
    message.clear();
    int i = 0;
    foreach (QString token, tokens)
    {
        if (i % 2 == 0)
        {
            message.append(token);
        }
        else
        {
            VariableController* vc = command->variableController();
            if (vc->isVariableExist(token))
            {
                message.append(QString::number(vc->currentValue(token)));
            }
            else
            {
                message.append(DELIMITER);
                message.append(token);
                message.append(DELIMITER);
            }
        }

        ++i;
    }

    addMessage(command->consoleTextColor(), message);
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
