#include "Headers/gui/application_finish_dialog.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/file_reader.h"
#include "Headers/logger/Logger.h"

#include <QTimer>
#include <QtWidgets>

ApplicationFinishDialog::ApplicationFinishDialog(QWidget * parent):
    QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint /*| Qt::WindowTitleHint*/);
    QGridLayout * layout = new QGridLayout(this);

    QLabel* label = new QLabel(this);
    label->setText(tr("Safe application closing..."));
    layout->addWidget(label, 0, 0);

    setLayout(layout);
    setWindowTitle(tr("Application exit"));
}

ApplicationFinishDialog::~ApplicationFinishDialog()
{

}

bool ApplicationFinishDialog::init()
{
    // TODO get "set initial state" cyclogram file name from configuration file data
    QDir dir(Cyclogram::defaultStorePath() + "set_initial_state.cgr");
    QString fileName = dir.absolutePath();

    mCyclogram = new Cyclogram(this);
    if (!mCyclogram->load(fileName))
    {
        LOG_ERROR(QString("Could not load file '%1'. Skipping safe application finish...").arg(fileName));
        return false;
    }

    connect(mCyclogram, SIGNAL(finished(const QString&)), this, SLOT(onCyclogramFinish(const QString&)));
    mCyclogram->run();
    return true;
}

void ApplicationFinishDialog::onCyclogramFinish(const QString& error)
{
    if (error.isEmpty())
    {
        LOG_INFO(QString("Set to safe state cyclogram succesfully executed"));
    }
    else
    {
        LOG_ERROR(QString("Set to safe state cyclogram finished with error: %1").arg(error));
    }

    close();
}
