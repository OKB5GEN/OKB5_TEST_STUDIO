#include "Headers/gui/modal_cyclogram_execution_dialog.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/file_reader.h"
#include "Headers/logger/Logger.h"
#include "Headers/logic/cyclogram_manager.h"
#include "Headers/gui/tools/cyclogram_console.h"

#include <QTimer>
#include <QtWidgets>

ModalCyclogramExecutionDialog::ModalCyclogramExecutionDialog(QWidget * parent):
    QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint /*| Qt::WindowTitleHint*/);
    QGridLayout * layout = new QGridLayout(this);

    mText = new QLabel(this);
    layout->addWidget(mText, 0, 0);

    setLayout(layout);
}

ModalCyclogramExecutionDialog::~ModalCyclogramExecutionDialog()
{

}

bool ModalCyclogramExecutionDialog::init(const QString& fileName, const QString& text, SystemState* systemState, CyclogramConsole* console)
{
    // Execute cyclogram file if exist
    QString fullFileName = Cyclogram::defaultStorePath() + fileName;
    bool ok = false;
    auto cyclogram = CyclogramManager::createCyclogram(fullFileName, &ok);

    if (ok)
    {
        LOG_INFO(text);
        mText->setText(text);
        cyclogram->setSystemState(systemState);

        mCyclogram = cyclogram;
        connect(cyclogram.data(), SIGNAL(finished(const QString&)), this, SLOT(onCyclogramFinish(const QString&)));

        connect(cyclogram.data(), SIGNAL(commandStarted(Command*)), console, SLOT(onCommandStarted(Command*)));
        connect(cyclogram.data(), SIGNAL(commandFinished(Command*)), console, SLOT(onCommandFinished(Command*)));

        cyclogram->run();
    }
    else
    {
        LOG_WARNING(QString("No valid '%1' cyclogram file found").arg(fileName));
        CyclogramManager::removeCyclogram(cyclogram);
        return false;
    }

    return true;
}

void ModalCyclogramExecutionDialog::onCyclogramFinish(const QString& error)
{
    if (error.isEmpty())
    {
        LOG_INFO(QString("Cyclogram finished OK"));
    }
    else
    {
        LOG_ERROR(QString("Cyclogram finished with error: %1").arg(error));
    }

    CyclogramManager::removeCyclogram(mCyclogram.lock());
    close();
}
