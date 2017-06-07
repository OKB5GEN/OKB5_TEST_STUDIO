#include <QtWidgets>

#include "Headers/gui/cyclogram/dialogs/subprogram_dialog.h"
#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/logic/commands/cmd_sub_program.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"
#include "Headers/file_reader.h"
#include "Headers/file_writer.h"
#include "Headers/gui/tools/cyclogram_chart_dialog.h"
#include "Headers/gui/cyclogram/variables_window.h"
#include "Headers/gui/cyclogram/dialogs/cyclogram_settings_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cmd_subprogram_edit_dialog.h"


namespace
{
    static const int SIZE_ADJUST = 100;
    static const int MAX_DIALOG_WIDTH = 1600;
    static const int MAX_DIALOG_HEIGHT = 800;
}

SubProgramDialog::SubProgramDialog(CmdSubProgram* command, QWidget * mainWindow):
    QDialog(mainWindow),
    mCommand(command),
    mVariablesWindow(Q_NULLPTR)
{
    mScrollArea = new QScrollArea(this);
    mScrollArea->setBackgroundRole(QPalette::Dark);

    setMaximumSize(QSize(MAX_DIALOG_WIDTH, MAX_DIALOG_HEIGHT));

    mCyclogramWidget = new CyclogramWidget(this);
    mCyclogramWidget->setMainWindow(mainWindow);
    mCyclogramWidget->setParentScrollArea(mScrollArea);
    mCyclogramWidget->load(mCommand->cyclogram());

    mScrollArea->setWidget(mCyclogramWidget);

    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    layout->addLayout(buttonLayout);

    QPushButton* saveBtn = new QPushButton(QIcon(":/resources/images/save"), tr("Save"), this);
    QPushButton* variablesBtn = new QPushButton(QIcon(":/resources/images/variable"), tr("Variables"), this);
    QPushButton* chartBtn = new QPushButton(QIcon(":/resources/images/monitor_auto"), tr("Chart"), this);
    QPushButton* deleteBtn = new QPushButton(QIcon(":/resources/images/delete_all"), tr("Delete"), this);
    QPushButton* cyclogramSettingsBtn = new QPushButton(QIcon(":/resources/images/settings"), tr("Cyclogram Settings"), this);
    QPushButton* commandSettingsBtn = new QPushButton(QIcon(":/resources/images/settings"), tr("Command Settings"), this);

    connect(saveBtn, SIGNAL(clicked(bool)), this, SLOT(onSaveClick()));
    connect(variablesBtn, SIGNAL(clicked(bool)), this, SLOT(onVariablesClick()));
    connect(chartBtn, SIGNAL(clicked(bool)), this, SLOT(onChartClick()));
    connect(deleteBtn, SIGNAL(clicked(bool)), mCyclogramWidget, SLOT(deleteSelectedItem()));
    connect(cyclogramSettingsBtn, SIGNAL(clicked(bool)), this, SLOT(onCyclogramSettingsClick()));
    connect(commandSettingsBtn, SIGNAL(clicked(bool)), this, SLOT(onCommandSettingsClick()));

    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(variablesBtn);
    buttonLayout->addWidget(chartBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addWidget(cyclogramSettingsBtn);
    buttonLayout->addWidget(commandSettingsBtn);
    buttonLayout->addStretch();

    layout->addWidget(mScrollArea);
    setLayout(layout);
    setAttribute(Qt::WA_DeleteOnClose);

    updateSize();
}

SubProgramDialog::~SubProgramDialog()
{

}

void SubProgramDialog::onSaveClick()
{
    QString fileName;

    if (mCommand->filePath().isEmpty())
    {
        fileName = QFileDialog::getSaveFileName(this, tr("Save cyclogram file"), Cyclogram::defaultStorePath(), tr("OKB5 Cyclogram Files (*.cgr)"));

        if (fileName.isEmpty())
        {
            return;
        }
    }
    else
    {
        fileName = Cyclogram::defaultStorePath() + mCommand->filePath();
    }

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
        if (mCommand->filePath().isEmpty())
        {
            QStringList tokens = fileName.split(Cyclogram::defaultStorePath());

            if (tokens.size() != 2)
            {
                LOG_ERROR(QString("Invalid directory. All cyclograms must be stored in %1 or its subfolders").arg(Cyclogram::defaultStorePath()));
                return;
            }

            mCommand->setFilePath(tokens.at(1), false);
        }
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
    CyclogramChartDialog* dialog = new CyclogramChartDialog(parentWidget());
    dialog->setWindowTitle(windowTitle());
    dialog->setCyclogram(mCommand->cyclogram());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void SubProgramDialog::onCyclogramSettingsClick()
{
    CyclogramSettingsDialog dialog(Q_NULLPTR);
    dialog.setCyclogram(mCommand->cyclogram());
    dialog.exec();
}

CyclogramWidget* SubProgramDialog::cyclogramWidget() const
{
    return mCyclogramWidget;
}

void SubProgramDialog::updateSize()
{
    mScrollArea->resize(mCyclogramWidget->size());

    QSize defaultSize = mScrollArea->size();
    defaultSize.setWidth(defaultSize.width() + SIZE_ADJUST);
    defaultSize.setHeight(defaultSize.height() + SIZE_ADJUST);
    resize(defaultSize);
}

void SubProgramDialog::onCommandSettingsClick()
{
    //TODO connect cyclogram & cyclogram widget signals
    // (file changed, can be saved, cyclogram command selected and can be deleted, variables window active)
    // QFileSystemWatcher - for outside file/directory changing detection etc

    QString filePathBefore = mCommand->filePath();

    CmdSubProgramEditDialog dialog(Q_NULLPTR);
    dialog.setCommand(mCommand, mCommand->cyclogram());
    dialog.exec();

    QString filePathAfter = mCommand->filePath();

    if (filePathBefore != filePathAfter)
    {
        mCyclogramWidget->load(mCommand->cyclogram());
        updateSize();
    }
}

CmdSubProgram* SubProgramDialog::command() const
{
    return mCommand;
}

void SubProgramDialog::onCommandTextChanged(const QString& newText)
{
    QString title = windowTitle();

    QString newTitle;
    QString delimiter = CyclogramWidget::delimiter();
    QStringList token = title.split(delimiter);
    if (token.empty())
    {
        newTitle = newText;
    }
    else
    {
        token.pop_back();

        foreach (QString str, token)
        {
            newTitle.append(str);
            newTitle.append(delimiter);
        }

        newTitle.append(newText);
    }

    setWindowTitle(newTitle);
    mCyclogramWidget->setWindowTitle(newTitle);
}

void SubProgramDialog::onParentWindowTitleChanged(const QString& newParentWindowTitle)
{
    QString delimiter = CyclogramWidget::delimiter();
    QString newTitle = newParentWindowTitle + delimiter + mCommand->text();
    setWindowTitle(newTitle);
    mCyclogramWidget->setWindowTitle(newTitle);
}
