#include <QtWidgets>

#include "Headers/gui/editor_window.h"
#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/gui/tools/cyclogram_chart_dialog.h"
#include "Headers/gui/modal_cyclogram_execution_dialog.h"
#include "Headers/gui/tools/app_console.h"
#include "Headers/gui/tools/app_settings_dialog.h"
#include "Headers/app_settings.h"
#include "Headers/gui/tools/cyclogram_console.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/gui/cyclogram/dialogs/cyclogram_end_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cyclogram_settings_dialog.h"
#include "Headers/gui/cyclogram/variables_window.h"
#include "Headers/system/system_state.h"
#include "Headers/logic/cyclogram_manager.h"

#include "Headers/file_reader.h"
#include "Headers/file_writer.h"

#include "Headers/logger/Logger.h"

#include "Headers/logic/commands/cmd_sub_program.h"
#include "Headers/gui/cyclogram/dialogs/subprogram_dialog.h"

namespace
{
    static const int TOOLBAR_ICON_SIZE = 64;
    static const QString SETTING_LAST_OPEN_FILE_DIR = "LastOpenFileDir";
    static const QString SETTING_LAST_SAVE_FILE_DIR = "LastSaveFileDir";
}

EditorWindow::EditorWindow():
    mVariablesWindow(Q_NULLPTR),
    mScaleFactor(1.0)
{
    mScrollArea = new QScrollArea(this);
    mSystemState = new SystemState(this);
    mCyclogramWidget = new CyclogramWidget(this);
    mCyclogramWidget->setMainWindow(this);

    mCyclogramWidget->setParentScrollArea(mScrollArea);

    mCyclogramConsole = new CyclogramConsole(this);

    mScrollArea->setBackgroundRole(QPalette::Dark);
    mScrollArea->setWidget(mCyclogramWidget);
    setCentralWidget(mScrollArea);

    createActions();
    createStatusBar();

    readSettings();

    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest, this, &EditorWindow::commitData);

    setNewCyclogram(CyclogramManager::createCyclogram());

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);
}

void EditorWindow::onApplicationStart()
{
    mSystemState->onApplicationStart();
    QString fileName = AppSettings::instance().settingValue(AppSettings::APP_START_CYCLOGRAM_FILE).toString();
    runModalCyclogram(fileName, tr("Running application start cyclogram..."));
}

void EditorWindow::closeEvent(QCloseEvent *event)
{
    auto cyclogram = mCyclogram.lock();

    if (cyclogram->state() == Cyclogram::RUNNING)
    {
        int button =  QMessageBox::question(this,
                                            tr("Application exit"),
                                            tr("Cyclogram is running. Would you like to stop it?"),
                                            QMessageBox::Ok,
                                            QMessageBox::Cancel);

        if (button == QMessageBox::Ok)
        {
            cyclogram->disconnect();
            cyclogram->stop();
        }
        else
        {
            event->ignore();
            return;
        }
    }

    if (maybeSave())
    {
        writeSettings();
        QString fileName = AppSettings::instance().settingValue(AppSettings::APP_FINISH_CYCLOGRAM_FILE).toString();
        runModalCyclogram(fileName, tr("Running application finish cyclogram..."));
        mSystemState->onApplicationFinish();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void EditorWindow::newFile()
{
    if (maybeSave())
    {
        CyclogramManager::clear();
        auto cyclogram = CyclogramManager::createCyclogram();

        setNewCyclogram(cyclogram);
        setCurrentFile(QString());

        cyclogram->setModified(true, true);
    }
}

void EditorWindow::openFile()
{
    if (maybeSave())
    {
        // try read last file open path
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        QString path = settings.value(SETTING_LAST_OPEN_FILE_DIR).toString();
        if (path.isEmpty())
        {
            path = QDir::currentPath();
        }

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open cyclogram file"), path, tr("OKB5 Cyclogram Files (*.cgr)"));
        if (!fileName.isEmpty())
        {
            // load cyclogram
            QStringList tokens = fileName.split(Cyclogram::defaultStorePath());

            if (tokens.size() != 2)
            {
                LOG_ERROR(QString("Invalid directory. All cyclograms must be stored in %1 or its subfolders").arg(Cyclogram::defaultStorePath()));
                return;
            }

            QString openPath = QFileInfo(fileName).absoluteDir().path();
            settings.setValue(SETTING_LAST_OPEN_FILE_DIR, openPath);
            loadFile(fileName);
        }
    }
}

bool EditorWindow::save()
{
    if (mCurFile.isEmpty())
    {
        return saveAs();
    }
    else
    {
        return saveFile(mCurFile);
    }
}

bool EditorWindow::saveAs()
{
    // try read last file save path
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString path = settings.value(SETTING_LAST_SAVE_FILE_DIR).toString();
    if (path.isEmpty())
    {
        path = QDir::currentPath();
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save cyclogram file"), path, tr("OKB5 Cyclogram Files (*.cgr)"));

    if (fileName.isEmpty())
    {
        return false;
    }

    return saveFile(fileName);

    //QFileDialog dialog(this);
    //dialog.setWindowModality(Qt::WindowModal);
    //dialog.setAcceptMode(QFileDialog::AcceptSave);
    //if (dialog.exec() != QDialog::Accepted)
    //{
    //    return false;
    //}

    //return saveFile(dialog.selectedFiles().first());
}

void EditorWindow::about()
{
   QMessageBox::about(this, tr("About Application"),
            tr("The <b>Application</b> example demonstrates how to "
               "write modern GUI applications using Qt, with a menu bar, "
               "toolbars, and a status bar."));
}

void EditorWindow::documentWasModified()
{
    auto cyclogram = mCyclogram.lock();
    setWindowModified(cyclogram->isModified());

    if (cyclogram->isModified())
    {
        emit documentSaved(false);
    }
}

void EditorWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));
    fileToolBar->setIconSize(QSize(TOOLBAR_ICON_SIZE, TOOLBAR_ICON_SIZE));

    QIcon newIcon = QIcon(":/resources/images/new");
    mNewAct = new QAction(newIcon, tr("&New"), this);
    mNewAct->setShortcuts(QKeySequence::New);
    mNewAct->setStatusTip(tr("Create a new cyclogram"));
    connect(mNewAct, &QAction::triggered, this, &EditorWindow::newFile);
    fileMenu->addAction(mNewAct);
    fileToolBar->addAction(mNewAct);

    QIcon openIcon = QIcon(":/resources/images/open");
    mOpenAct = new QAction(openIcon, tr("&Open..."), this);
    mOpenAct->setShortcuts(QKeySequence::Open);
    mOpenAct->setStatusTip(tr("Open an existing file"));
    connect(mOpenAct, &QAction::triggered, this, &EditorWindow::openFile);
    fileMenu->addAction(mOpenAct);
    fileToolBar->addAction(mOpenAct);

    QIcon saveIcon = QIcon(":/resources/images/save");
    mSaveAct = new QAction(saveIcon, tr("&Save"), this);
    mSaveAct->setShortcuts(QKeySequence::Save);
    mSaveAct->setStatusTip(tr("Save the document to disk"));
    connect(mSaveAct, &QAction::triggered, this, &EditorWindow::save);
    connect(this, SIGNAL(documentSaved(bool)), mSaveAct, SLOT(setDisabled(bool)));
    fileMenu->addAction(mSaveAct);
    fileToolBar->addAction(mSaveAct);

    QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this, &EditorWindow::saveAs);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new file name"));

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));

    /*
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolBar = addToolBar(tr("Edit"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &EditorWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
*/
    QMenu *runMenu = menuBar()->addMenu(tr("Cyclogram"));
    QToolBar *runToolBar = addToolBar(tr("Cyclogram"));
    runToolBar->setIconSize(QSize(TOOLBAR_ICON_SIZE, TOOLBAR_ICON_SIZE));

#ifdef ENABLE_CYCLOGRAM_PAUSE
    mPlayIcon = QIcon(":/resources/images/play");
    mPauseIcon = QIcon(":/resources/images/pause");


    mRunAct = new QAction(mPlayIcon, tr("Run"), this);
#else
    mRunAct = new QAction(QIcon(":/resources/images/play"), tr("Run"), this);
#endif

    //mRunAct->setShortcuts(QKeySequence::New);
    mRunAct->setStatusTip(tr("Execute cyclogram"));
    connect(mRunAct, &QAction::triggered, this, &EditorWindow::runCyclogram);
    runMenu->addAction(mRunAct);
    runToolBar->addAction(mRunAct);

    QIcon stopIcon = QIcon(":/resources/images/stop");
    mStopAct = new QAction(stopIcon, tr("Stop"), this);
    //stopAct->setShortcuts(QKeySequence::New);
    mStopAct->setStatusTip(tr("Stop cyclogram execution"));
    connect(mStopAct, &QAction::triggered, this, &EditorWindow::stopCyclogram);
    runMenu->addAction(mStopAct);
    runToolBar->addAction(mStopAct);

    runMenu->addSeparator();

    QIcon settingsIcon = QIcon(":/resources/images/settings");
    mSettingsAct = new QAction(settingsIcon, tr("Settings"), this);
    mSettingsAct->setStatusTip(tr("Cyclogram settings"));
    connect(mSettingsAct, &QAction::triggered, this, &EditorWindow::showCyclogramSettings);
    runMenu->addAction(mSettingsAct);
    runToolBar->addAction(mSettingsAct);

    const QIcon addVariablesIcon = QIcon(":/resources/images/variable");
    QAction *addVariablesAct = new QAction(addVariablesIcon, tr("Variables"), this);
    addVariablesAct->setStatusTip(tr("Show cyclogram variables"));
    connect(addVariablesAct, &QAction::triggered, this, &EditorWindow::addVariablesMonitor);
    runMenu->addAction(addVariablesAct);
    runToolBar->addAction(addVariablesAct);

    stopCyclogram();

//    QMenu *monitorMenu = menuBar()->addMenu(tr("&Monitor"));
//    QToolBar *monitorToolBar = addToolBar(tr("Monitor"));
//    monitorToolBar->setIconSize(QSize(TOOLBAR_ICON_SIZE, TOOLBAR_ICON_SIZE));

//    const QIcon addManualMonitorIcon = QIcon(":/resources/images/monitor_manual");
//    QAction *addManualMonitorAct = new QAction(addManualMonitorIcon, tr("Make data snapshot"), this);
//    addManualMonitorAct->setStatusTip(tr("Make data snapshot"));
//    connect(addManualMonitorAct, &QAction::triggered, this, &EditorWindow::makeDataSnapshot);
//    monitorMenu->addAction(addManualMonitorAct);
//    monitorToolBar->addAction(addManualMonitorAct);

//    const QIcon addAutoMonitorIcon = QIcon(":/resources/images/monitor_auto");
//    QAction *addAutoMonitorAct = new QAction(addAutoMonitorIcon, tr("Add auto monitor"), this);
//    addAutoMonitorAct->setStatusTip(tr("Add auto parameter monitor"));
//    connect(addAutoMonitorAct, &QAction::triggered, this, &EditorWindow::addChartWidget);
//    monitorMenu->addAction(addAutoMonitorAct);
//    monitorToolBar->addAction(addAutoMonitorAct);

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));

    // add cyclogram console
    QDockWidget *cyclogramConsole = new QDockWidget(tr("Cyclogram console"), this);
    cyclogramConsole->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    cyclogramConsole->setWidget(mCyclogramConsole);
    addDockWidget(Qt::BottomDockWidgetArea, cyclogramConsole);
    toolsMenu->addAction(cyclogramConsole->toggleViewAction());

    // add application console
    QDockWidget *appConsole = new QDockWidget(tr("Application console"), this);
    appConsole->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    AppConsole* console = new AppConsole(this);
    appConsole->setWidget(console);
    addDockWidget(Qt::BottomDockWidgetArea, appConsole);
    toolsMenu->addAction(appConsole->toggleViewAction());

    toolsMenu->addSeparator();

    QAction *settingsAct = toolsMenu->addAction(tr("Settings"), this, &EditorWindow::onSettings);
    settingsAct->setStatusTip(tr("Application settings"));
}

void EditorWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void EditorWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty())
    {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2, (availableGeometry.height() - height()) / 2);
    }
    else
    {
        restoreGeometry(geometry);
    }
}

void EditorWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

bool EditorWindow::maybeSave()
{
    auto cyclogram = mCyclogram.lock();
    if (!cyclogram->isModified())
    {
        return true;
    }

    const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("OKB5 Test Studio"),
                               tr("The cyclogram has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret)
    {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }

    return true;
}

void EditorWindow::loadFile(const QString &fileName)
{
    CyclogramManager::clear();

    bool ok = false;
    auto cyclogram = CyclogramManager::createCyclogram(fileName, &ok);

    setNewCyclogram(cyclogram);

    if (!ok)
    {
        QMessageBox::warning(this, tr("OKB5 Test Studio"),
                                   tr("File '%1' not loaded.").
                                   arg(QDir::toNativeSeparators(fileName)));
    }
    else
    {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File loaded"), 2000);
    }

    mSaveAct->setDisabled(true);
}

bool EditorWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("OKB5 Test Studio"),
                                   tr("Cannot write file %1:\n%2.").
                                   arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    FileWriter writer(mCyclogram.lock());
    if (writer.writeFile(&file))
    {
        // save last save dir
        QSettings settings;
        QString savePath = QFileInfo(fileName).absoluteDir().path();
        settings.setValue(SETTING_LAST_SAVE_FILE_DIR, savePath);

        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File saved"), 2000);
        emit documentSaved(true);
        return true;
    }

    return false;
}

void EditorWindow::setCurrentFile(const QString &fileName)
{
    mCurFile = fileName;
    auto cyclogram = mCyclogram.lock();
    cyclogram->setModified(false, false);
    setWindowModified(false);

    QString shownName = mCurFile;
    if (mCurFile.isEmpty())
    {
        shownName = "New file";
    }

    shownName += "[*]";
    setWindowFilePath(shownName);
    setWindowTitle(shownName);
}

QString EditorWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void EditorWindow::runCyclogram()
{
    mSnapshotsCouner = 0;
    auto cyclogram = mCyclogram.lock();

    Command* errorCmd = cyclogram->validate();
    if (errorCmd)
    {
        mCyclogramWidget->showValidationError(errorCmd);
        LOG_ERROR(QString("Cyclogram validation failed"));
        return;
    }

//    QString fileName = AppSettings::instance().settingValue(AppSettings::CYCLOGRAM_START_CYCLOGRAM_FILE).toString();
//    runModalCyclogram(fileName, tr("Running pre-execution cyclogram..."));

#ifdef ENABLE_CYCLOGRAM_PAUSE
    if (mCyclogram->state() == Cyclogram::STOPPED)
    {
        mRunAct->setIcon(mPauseIcon);
        mRunAct->setStatusTip(tr("Pause cyclogram execution"));
        mCyclogram->run();
    }
    else if (mCyclogram->state() == Cyclogram::RUNNING)
    {
        mRunAct->setIcon(mPlayIcon);
        mRunAct->setStatusTip(tr("Execute cyclogram"));
        mCyclogram->pause();
    }
    else if (mCyclogram->state() == Cyclogram::PAUSED)
    {
        mRunAct->setIcon(mPauseIcon);
        mRunAct->setStatusTip(tr("Pause cyclogram execution"));
        mCyclogram->resume();
    }
#else
    mRunAct->setEnabled(false);
    cyclogram->run();
#endif

    mStopAct->setEnabled(true);
}

void EditorWindow::stopCyclogram()
{
    auto cyclogram = mCyclogram.lock();

    if (QObject::sender() == mStopAct) // stop only by button signal
    {
        cyclogram->stop();
    }

#ifdef ENABLE_CYCLOGRAM_PAUSE
    mRunAct->setIcon(mPlayIcon);
#endif
    mRunAct->setStatusTip(tr("Execute cyclogram"));
    mStopAct->setEnabled(false);

    //mRunOneCmdAct->setEnabled(true);
    mRunAct->setEnabled(true);
}

void EditorWindow::showCyclogramSettings()
{
    CyclogramSettingsDialog dialog(Q_NULLPTR);
    dialog.setCyclogram(mCyclogram.lock());
    dialog.exec();
}

void EditorWindow::addVariablesMonitor()
{
    if (!mVariablesWindow)
    {
        mVariablesWindow = new VariablesWindow(this);
    }

    mVariablesWindow->setCyclogram(mCyclogram);
    mVariablesWindow->show();
}

void EditorWindow::makeDataSnapshot()
{
    ++mSnapshotsCouner;
    auto cyclogram = mCyclogram.lock();
    cyclogram->variableController()->makeDataSnapshot(QString("Label %1").arg(mSnapshotsCouner));

    int TODO; // по идее надо делать снэпшот, не главной циклограммы, а активной подпрограммы, которая самая верхняя по стеку
    // хотя по большому счету важны не реальные значения переменных, а просто момент, что "вот в этот момент произошла
    // неведомая х*ня, которую надо смотреть по логам"
}

void EditorWindow::addChartWidget()
{
    CyclogramChartDialog* dialog = new CyclogramChartDialog(this);

    dialog->setCyclogram(mCyclogram);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void EditorWindow::onCyclogramFinish(const QString& errorText)
{
    CyclogramEndDialog * dialog = new CyclogramEndDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    stopCyclogram();

    dialog->setCyclogram(mCyclogram);

    if (!errorText.isEmpty())
    {
        dialog->setText(errorText);
    }
    else
    {
        dialog->setText(tr("Cyclogram execution finished"));
    }

    dialog->exec();
}

void EditorWindow::runModalCyclogram(const QString& shortFileName, const QString& text)
{
    if (shortFileName.isEmpty())
    {
        return;
    }

    ModalCyclogramExecutionDialog dialog(Q_NULLPTR);
    if (dialog.init(shortFileName, text, mSystemState))
    {
        setEnabled(false);
        dialog.exec();
        setEnabled(true);
    }
}

void EditorWindow::onCyclogramStateChanged(int state)
{
    auto cyclogram = mCyclogram.lock();
    mOpenAct->setEnabled(state == Cyclogram::STOPPED);
    mNewAct->setEnabled(state == Cyclogram::STOPPED);
    mSaveAct->setEnabled(state == Cyclogram::STOPPED && cyclogram->isModified());

#ifdef ENABLE_CYCLOGRAM_PAUSE
    if (state == Cyclogram::PAUSED)
    {
        mRunAct->setIcon(mPlayIcon);
        mRunAct->setStatusTip(tr("Execute cyclogram"));
    }
#endif

    if (state == Cyclogram::STOPPED)
    {
        QString fileName = cyclogram->setting(Cyclogram::SETTING_CLEANUP_CYCLOGRAM).toString();
        if (fileName.isEmpty())
        {
            return;
        }

        runModalCyclogram(fileName, tr("Running post-execution cyclogram..."));
    }
}

void EditorWindow::commitData(QSessionManager &manager)
{
    if (manager.allowsInteraction())
    {
        if (!maybeSave())
        {
            manager.cancel();
        }
    }
    else
    {
        auto cyclogram = mCyclogram.lock();
        // Non-interactive: save without asking
        if (cyclogram->isModified())
        {
            save();
        }
    }
}

void EditorWindow::setNewCyclogram(QSharedPointer<Cyclogram> cyclogram)
{
    mOpenedSubprogramDialogs.clear(); // all dialogs will be closed by destroyed() command signal

    mCyclogramConsole->clear();

    mCyclogram = cyclogram;

    mCyclogramWidget->clear();
    cyclogram->setMainCyclogram(true);

    connect(cyclogram.data(), SIGNAL(finished(const QString&)), this, SLOT(onCyclogramFinish(const QString&)));
    connect(cyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
    connect(cyclogram.data(), SIGNAL(modified()), this, SLOT(documentWasModified()));

    connect(cyclogram.data(), SIGNAL(commandStarted(Command*)), mCyclogramConsole, SLOT(onCommandStarted(Command*)));
    connect(cyclogram.data(), SIGNAL(commandFinished(Command*)), mCyclogramConsole, SLOT(onCommandFinished(Command*)));

    cyclogram->setSystemState(mSystemState);

    mCyclogramWidget->load(cyclogram);

}

void EditorWindow::onSettings()
{
    AppSettingsDialog dialog(Q_NULLPTR);
    dialog.exec();
}

SubProgramDialog* EditorWindow::subprogramDialog(CmdSubProgram* command) const
{
    return qobject_cast<SubProgramDialog*>(mOpenedSubprogramDialogs.value(command, Q_NULLPTR));
}

void EditorWindow::addSuprogramDialog(CmdSubProgram* command, SubProgramDialog* dialog)
{
    Q_ASSERT(subprogramDialog(command) == Q_NULLPTR);

    connect(command, SIGNAL(destroyed(QObject*)), this, SLOT(onSubprogramDestroyed(QObject*)));
    connect(dialog, SIGNAL(destroyed(QObject*)), this, SLOT(onSubprogramDialogDestroyed(QObject*)));

    mOpenedSubprogramDialogs[command] = dialog;
}

void EditorWindow::onSubprogramDestroyed(QObject* object)
{
    // command deleted by the user
    SubProgramDialog* dialog = qobject_cast<SubProgramDialog*>(mOpenedSubprogramDialogs.take(object)); // close dialog and remove it from container
    if (dialog)
    {
        disconnect(dialog, SIGNAL(destroyed(QObject*)), this, SLOT(onSubprogramDialogDestroyed(QObject*)));
        dialog->close();
    }
}

void EditorWindow::onSubprogramDialogDestroyed(QObject* object)
{
    // dialog closed by the user
    QObject* key = mOpenedSubprogramDialogs.key(object, Q_NULLPTR);
    if (!key)
    {
        return;
    }

    mOpenedSubprogramDialogs.take(key);
}
