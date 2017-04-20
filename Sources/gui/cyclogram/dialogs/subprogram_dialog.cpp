#include <QtWidgets>

#include "Headers/gui/cyclogram/dialogs/subprogram_dialog.h"
#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/logic/commands/cmd_sub_program.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"
#include "Headers/file_reader.h"
#include "Headers/file_writer.h"
#include "Headers/gui/tools/monitor_auto.h"
#include "Headers/gui/cyclogram/variables_window.h"

SubProgramDialog::SubProgramDialog(CmdSubProgram* command, QWidget * parent):
    QDialog(parent),
    mCommand(command),
    mVariablesWindow(Q_NULLPTR)
{
    mScrollArea = new QScrollArea(this);
    setMaximumSize(QSize(1600, 800));

    mCyclogramWidget = new CyclogramWidget(this);
    mCyclogramWidget->setDialogParent(parent);
    mCyclogramWidget->load(mCommand->cyclogram());

    connect(mCommand->cyclogram().data(), SIGNAL(destroyed(QObject*)), this, SLOT(close()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    layout->addLayout(buttonLayout);

    QPushButton* saveBtn = new QPushButton(QIcon(":/images/save"), tr("Save"), this);
    QPushButton* variablesBtn = new QPushButton(QIcon(":/images/variable"), tr("Variables"), this);
    QPushButton* chartBtn = new QPushButton(QIcon(":/images/monitor_auto"), tr("Chart"), this);
    QPushButton* deleteBtn = new QPushButton(QIcon(":/images/delete_all"), tr("Delete"), this);

    connect(saveBtn, SIGNAL(clicked(bool)), this, SLOT(onSaveClick()));
    connect(variablesBtn, SIGNAL(clicked(bool)), this, SLOT(onVariablesClick()));
    connect(chartBtn, SIGNAL(clicked(bool)), this, SLOT(onChartClick()));
    connect(deleteBtn, SIGNAL(clicked(bool)), mCyclogramWidget, SLOT(deleteSelectedItem()));

    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(variablesBtn);
    buttonLayout->addWidget(chartBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addStretch();

    mScrollArea->setBackgroundRole(QPalette::Dark);
    mScrollArea->setWidget(mCyclogramWidget);
    mScrollArea->resize(mCyclogramWidget->size()); //TODO too big size case

    layout->addWidget(mScrollArea);
    setLayout(layout);
    setAttribute(Qt::WA_DeleteOnClose);
    resize(mScrollArea->size());

    //TODO connect cyclogram & cyclogram widget signals
    // (file changed, can be saved, cyclogram command selected and can be deleted, variables window active)
    // QFileSystemWatcher - for outside file/directory changing detection etc
}

SubProgramDialog::~SubProgramDialog()
{

}

void SubProgramDialog::onSaveClick()
{
    QString fileName = Cyclogram::defaultStorePath() + mCommand->filePath();
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("OKB5 Test Studio"),
                                   tr("Cannot write file %1:\n%2.").
                                   arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    FileWriter writer(mCommand->cyclogram());
    if (writer.writeFile(&file))
    {
        LOG_INFO(QString("File '%1' saved").arg(fileName));
    }
    else
    {
        LOG_ERROR(QString("File '%1' not saved!").arg(fileName));
    }
}

void SubProgramDialog::onVariablesClick()
{
    if (!mVariablesWindow)
    {
        mVariablesWindow = new VariablesWindow(this);
        mVariablesWindow->setCyclogram(mCommand->cyclogram());
        mVariablesWindow->setWindowTitle(windowTitle());
    }

    mVariablesWindow->show();
}

void SubProgramDialog::onChartClick()
{
    MonitorAuto* dialog = new MonitorAuto(parentWidget());
    dialog->setWindowTitle(windowTitle());
    dialog->setCyclogram(mCommand->cyclogram());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}
