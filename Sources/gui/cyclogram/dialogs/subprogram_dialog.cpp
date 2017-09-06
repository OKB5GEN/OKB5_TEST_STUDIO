#include <QtWidgets>

#include "Headers/app_settings.h"
#include "Headers/gui/cyclogram/dialogs/subprogram_dialog.h"
#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/gui/cyclogram/shape_item.h"
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
#include "Headers/clipboard.h"


namespace
{
    static const int SIZE_ADJUST = 100;
    static const int DEFAULT_MAX_DIALOG_WIDTH = 1600;
    static const int DEFAULT_MAX_DIALOG_HEIGHT = 800;
}

SubProgramDialog::SubProgramDialog(CmdSubProgram* command, QSharedPointer<Cyclogram> callingCyclogram, QWidget* mainWindow, QSharedPointer<Clipboard> clipboard):
    QDialog(mainWindow),
    mCommand(command)
{
    connect(mCommand, SIGNAL(cyclogramChanged()), this, SLOT(reload()));

    mCallingCyclogram = callingCyclogram;
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    mScrollArea = new QScrollArea(this);
    mScrollArea->setBackgroundRole(QPalette::Dark);

    mCyclogramWidget = new CyclogramWidget(this);
    mCyclogramWidget->setMainWindow(mainWindow);
    mCyclogramWidget->setParentScrollArea(mScrollArea);
    mCyclogramWidget->setClipboard(clipboard);
    mCyclogramWidget->load(mCommand->cyclogram());

    mScrollArea->setWidget(mCyclogramWidget);

    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    layout->addLayout(buttonLayout);

    mSaveBtn = new QPushButton(QIcon(":/resources/images/save"), tr("Save"), this);
    mVariablesBtn = new QPushButton(QIcon(":/resources/images/variable"), tr("Variables"), this);
    mChartBtn = new QPushButton(QIcon(":/resources/images/monitor_auto"), tr("Chart"), this);
    mDeleteBtn = new QPushButton(QIcon(":/resources/images/delete_all"), tr("Delete"), this);
    mSettingsBtn = new QPushButton(QIcon(":/resources/images/settings"), tr("Settings"), this);

    connect(mSaveBtn, SIGNAL(clicked(bool)), this, SLOT(onSaveClick()));
    connect(mVariablesBtn, SIGNAL(clicked(bool)), this, SLOT(onVariablesClick()));
    connect(mChartBtn, SIGNAL(clicked(bool)), this, SLOT(onChartClick()));
    connect(mDeleteBtn, SIGNAL(clicked(bool)), mCyclogramWidget, SLOT(deleteSelectedItem()));
    connect(mSettingsBtn, SIGNAL(clicked(bool)), this, SLOT(onSettingsClick()));

    connect(mCommand->cyclogram().data(), SIGNAL(modified()), this, SLOT(onCyclogramModified()));
    connect(mCommand->cyclogram().data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
    connect(mCyclogramWidget, SIGNAL(selectionChanged(ShapeItem*)), this, SLOT(onCyclogramSelectionChanged(ShapeItem*)));

    buttonLayout->addWidget(mSaveBtn);
    buttonLayout->addWidget(mVariablesBtn);
    buttonLayout->addWidget(mChartBtn);
    buttonLayout->addWidget(mDeleteBtn);
    buttonLayout->addWidget(mSettingsBtn);
    buttonLayout->addStretch();

    layout->addWidget(mScrollArea);
    setLayout(layout);
    setAttribute(Qt::WA_DeleteOnClose);

    updateSize();

    QSize size = this->size();
    if (size.width() > DEFAULT_MAX_DIALOG_WIDTH || size.height() > DEFAULT_MAX_DIALOG_HEIGHT)
    {
        resize(QSize(qMin(size.width(), DEFAULT_MAX_DIALOG_WIDTH), qMin(size.height(), DEFAULT_MAX_DIALOG_HEIGHT)));
    }

    onCyclogramModified();
    onCyclogramSelectionChanged(mCyclogramWidget->selectedItem());
    onCyclogramStateChanged(mCommand->cyclogram()->state());
}

SubProgramDialog::~SubProgramDialog()
{

}

bool SubProgramDialog::onSaveClick()
{
    QString fileName;

    if (mCommand->filePath().isEmpty())
    {
        fileName = QFileDialog::getSaveFileName(this, tr("Save cyclogram file"), Cyclogram::defaultStorePath(), tr("OKB5 Cyclogram Files (*%1)").arg(AppSettings::extension()));

        if (fileName.isEmpty())
        {
            return false;
        }
    }
    else
    {
        fileName = Cyclogram::defaultStorePath() + mCommand->filePath();
    }

    bool fileWasNotExist = !QFileInfo::exists(fileName);

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("OKB5 Test Studio"),
                                   tr("Cannot write file %1:\n%2.").
                                   arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    mCommand->beforeSave();

    FileWriter writer(mCommand->cyclogram());
    if (writer.writeFile(&file))
    {
        LOG_INFO(QString("File '%1' saved").arg(fileName));

        mCommand->cyclogram()->setModified(false, true, false);

        if (mCommand->filePath().isEmpty())
        {
            QStringList tokens = fileName.split(Cyclogram::defaultStorePath());

            if (tokens.size() != 2)
            {
                LOG_ERROR(QString("Invalid directory. All cyclograms must be stored in %1 or its subfolders").arg(Cyclogram::defaultStorePath()));
                return false;
            }

            mCommand->setFilePath(tokens.at(1), false);
        }

        if (fileWasNotExist) // by default file reload is not needed, but if file was not exist before save, force its reload
        {
            mCommand->setLoaded(true);
        }
    }
    else
    {
        LOG_ERROR(QString("File '%1' not saved!").arg(fileName));
        return false;
    }

    return true;
}

void SubProgramDialog::onVariablesClick()
{
    VariablesWindow variablesWindow(this);
    variablesWindow.setCyclogram(mCommand->cyclogram());
    variablesWindow.setWindowTitle(windowTitle());
    variablesWindow.exec();
}

void SubProgramDialog::onChartClick()
{
    CyclogramChartDialog* dialog = new CyclogramChartDialog(parentWidget());
    dialog->setWindowTitle(windowTitle());
    dialog->setCyclogram(mCommand->cyclogram());
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(this, SIGNAL(windowTitleChanged(const QString&)), dialog, SLOT(setWindowTitle(const QString&)));
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
    //mScrollArea->resize(mCyclogramWidget->size());

    QSize defaultSize = mCyclogramWidget->size();//mScrollArea->size();
    defaultSize.setWidth(defaultSize.width() + SIZE_ADJUST);
    defaultSize.setHeight(defaultSize.height() + SIZE_ADJUST);
    resize(defaultSize);
}

void SubProgramDialog::onSettingsClick()
{
    QString filePathBefore = mCommand->filePath();

    CmdSubProgramEditDialog dialog(Q_NULLPTR);
    dialog.setCommand(mCommand, mCallingCyclogram.lock());
    dialog.exec();

    QString filePathAfter = mCommand->filePath();

    if (filePathBefore != filePathAfter)
    {
        reload();
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

    updateTitle(newTitle);
}

void SubProgramDialog::onParentWindowTitleChanged(const QString& newParentWindowTitle)
{
    QString delimiter = CyclogramWidget::delimiter();
    QString newTitle = newParentWindowTitle + delimiter + mCommand->text();
    updateTitle(newTitle);
}

void SubProgramDialog::updateTitle(const QString &newTitle)
{
    setWindowTitle(newTitle + "[*]");
    mCyclogramWidget->setWindowTitle(newTitle);
    onCyclogramModified();
}

void SubProgramDialog::onCyclogramModified()
{
    bool isModified = mCommand->cyclogram()->isModified();
    setWindowModified(isModified);
    mSaveBtn->setEnabled(isModified);
}

void SubProgramDialog::onCyclogramSelectionChanged(ShapeItem* item)
{
    mDeleteBtn->setEnabled(item != Q_NULLPTR);
}

void SubProgramDialog::closeEvent(QCloseEvent *event)
{
    bool isModified = isWindowModified();

    if (!isModified)
    {
        event->accept();
        return;
    }

    const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Subprogram window close"),
                               tr("The cyclogram has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
    {
        if (onSaveClick())
        {
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else if (ret == QMessageBox::Discard)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void SubProgramDialog::save()
{
    if (command()->cyclogram()->isModified())
    {
        onSaveClick();
    }
}

void SubProgramDialog::reload()
{
    mCyclogramWidget->load(mCommand->cyclogram());
    updateSize();
}

void SubProgramDialog::onCyclogramStateChanged(int state)
{
    //mSaveBtn;
    //mChartBtn;
    mVariablesBtn->setEnabled(state == Cyclogram::IDLE);
    mDeleteBtn->setEnabled(state == Cyclogram::IDLE);
    mSettingsBtn->setEnabled(state == Cyclogram::IDLE);
}
