#include <QtWidgets>

#include "Headers/gui/editor_window.h"
#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/gui/tools/cyclogram_chart_dialog.h"
#include "Headers/gui/modal_cyclogram_execution_dialog.h"
#include "Headers/gui/tools/app_console.h"
#include "Headers/gui/tools/app_settings_dialog.h"
#include "Headers/app_settings.h"
#include "Headers/gui/save_before_run_dialog.h"
#include "Headers/gui/tools/cyclogram_console.h"
#include "Headers/gui/tools/commands_edit_toolbar.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/gui/cyclogram/dialogs/cyclogram_end_dialog.h"
#include "Headers/gui/cyclogram/dialogs/cyclogram_settings_dialog.h"
#include "Headers/gui/cyclogram/variables_window.h"
#include "Headers/system/system_state.h"
#include "Headers/logic/cyclogram_manager.h"
#include "Headers/clipboard.h"

#include "Headers/file_reader.h"
#include "Headers/file_writer.h"

#include "Headers/logger/Logger.h"

#include "Headers/logic/commands/cmd_sub_program.h"
#include "Headers/gui/cyclogram/dialogs/subprogram_dialog.h"

namespace
{
    static const int TOOLBAR_ICON_SIZE = 48;
    static const QString SETTING_LAST_OPEN_FILE_DIR = "LastOpenFileDir";
    static const QString SETTING_LAST_SAVE_FILE_DIR = "LastSaveFileDir";
    static const QString LAST_OPENED_FILE_NAME = "LastOpenedFileName";
    static const QString SILENT_SAVE_BEFORE_START_FLAG = "SaveBeforeStart";
}

static inline QString recentFilesKey() { return QStringLiteral("recentFileList"); }
static inline QString fileKey() { return QStringLiteral("file"); }

static QStringList readRecentFiles(QSettings &settings)
{
    QStringList result;
    const int count = settings.beginReadArray(recentFilesKey());
    for (int i = 0; i < count; ++i)
    {
        settings.setArrayIndex(i);
        result.append(settings.value(fileKey()).toString());
    }

    settings.endArray();
    return result;
}

static void writeRecentFiles(const QStringList &files, QSettings &settings)
{
    const int count = files.size();
    settings.beginWriteArray(recentFilesKey());
    for (int i = 0; i < count; ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue(fileKey(), files.at(i));
    }

    settings.endArray();
}

EditorWindow::EditorWindow():
    mScaleFactor(1.0),
    mCommandsEditToolbar(Q_NULLPTR),
    mNeedHashUpdate(false)
{
    QString title = tr("New file");
    title += "[*]";
    setWindowTitle(title);

    mClipboard.reset(new Clipboard());

    mFileWatcher = new QFileSystemWatcher(this);
    connect(mFileWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(reloadCyclogram()));

    mScrollArea = new QScrollArea(this);
    mSystemState = new SystemState(this);

    mClipboard->setSystemState(mSystemState);

    mCyclogramWidget = new CyclogramWidget(this);
    mCyclogramWidget->setMainWindow(this);
    mCyclogramWidget->setParentScrollArea(mScrollArea);
    mCyclogramWidget->setClipboard(mClipboard);

    mCyclogramConsole = new CyclogramConsole(this);

    mScrollArea->setBackgroundRole(QPalette::Dark);
    mScrollArea->setWidget(mCyclogramWidget);
    setCentralWidget(mScrollArea);

    createActions();
    createStatusBar();

    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest, this, &EditorWindow::commitData);

    readSettings();

    setUnifiedTitleAndToolBarOnMac(true);
}

void EditorWindow::onApplicationStart()
{
    mSystemState->onApplicationStart();
    QString fileName = AppSettings::instance().settingValue(AppSettings::APP_START_CYCLOGRAM_FILE).toString();
    runModalCyclogram(fileName, tr("Running application start cyclogram..."));
}

void EditorWindow::openExistingFile()
{
    openFile(QString(""));
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

    int ret = 0;
    if (maybeSave(&ret))
    {
        writeSettings();
        QString fileName = AppSettings::instance().settingValue(AppSettings::APP_FINISH_CYCLOGRAM_FILE).toString();
        runModalCyclogram(fileName, tr("Running application finish cyclogram..."));
        mSystemState->onApplicationFinish();

        int options = (ret == QMessageBox::Save) ? EditorWindow::Save : 0;
        closeAll(options);

        CyclogramManager::clear();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void EditorWindow::updateSubprogramDialogs(int updateOption)
{
    int maxDepth = -1;
    QList<int> depths;

    //TODO optimize
    foreach (QObject* object, mOpenedSubprogramDialogs)
    {
        SubProgramDialog* dialog = qobject_cast<SubProgramDialog*>(object);
        QString title = dialog->windowTitle();
        QStringList token = title.split(CyclogramWidget::delimiter());
        int depth = token.size();

        depths.append(depth);

        if (depth > maxDepth)
        {
            maxDepth = depth;
        }
    }

    QSet<int> set = depths.toSet();
    QList<int> sorted = QList<int>::fromSet(set);
    qSort(sorted);

    auto dialogs = mOpenedSubprogramDialogs.values();

    while (!sorted.empty())
    {
        int depth = sorted.back();
        for (auto it = dialogs.begin(); it != dialogs.end();)
        {
            SubProgramDialog* dialog = qobject_cast<SubProgramDialog*>(*it);
            QString title = dialog->windowTitle();
            QStringList token = title.split(CyclogramWidget::delimiter());

            if (token.size() == depth)
            {
                if ((updateOption & EditorWindow::Save) != 0)
                {
                    dialog->save();
                }
                else
                {
                    dialog->command()->cyclogram()->setModified(false, false, true);
                    dialog->setWindowModified(false);
                }

                if ((updateOption & EditorWindow::Close) != 0)
                {
                    dialog->close();
                }

                it = dialogs.erase(it);
                continue;
            }

            ++it;
        }

        sorted.pop_back();
    }
}

void EditorWindow::closeAll(int options)
{
    updateSubprogramDialogs(EditorWindow::Close | options);
    CyclogramManager::clear();
    mCyclogramWidget->clear();
}

void EditorWindow::newFile()
{
    int ret = 0;
    if (!maybeSave(&ret))
    {
        return;
    }

    int options = (ret == QMessageBox::Save) ? EditorWindow::Save : 0;
    closeAll(options);

    auto cyclogram = CyclogramManager::createCyclogram();

    setNewCyclogram(cyclogram);
    setCurrentFile(QString());

    saveLastOpenedFile(QString());

    cyclogram->setModified(true, true, false);
}

void EditorWindow::openFile(const QString& name)
{
    int ret = 0;
    if (!maybeSave(&ret))
    {
        return;
    }

    QString fileName = name;
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString path = settings.value(SETTING_LAST_OPEN_FILE_DIR).toString();
    if (path.isEmpty())
    {
        path = QDir::currentPath();
    }

    if (fileName.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(this, tr("Open cyclogram file"), path, tr("OKB5 Cyclogram Files (*%1)").arg(AppSettings::extension()));
    }

    if (fileName.isEmpty())
    {
        return;
    }

    // load cyclogram
    QStringList tokens = fileName.split(Cyclogram::defaultStorePath());

    if (tokens.size() != 2)
    {
        LOG_ERROR(QString("Invalid directory. All cyclograms must be stored in %1 or its subfolders").arg(Cyclogram::defaultStorePath()));
        return;
    }

    int options = (ret == QMessageBox::Save) ? EditorWindow::Save : 0;

    QString openPath = QFileInfo(fileName).absoluteDir().path();
    settings.setValue(SETTING_LAST_OPEN_FILE_DIR, openPath);
    loadFile(fileName, options);
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

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save cyclogram file"), path, tr("OKB5 Cyclogram Files (*%1)").arg(AppSettings::extension()));

    if (fileName.isEmpty())
    {
        return false;
    }

    bool result = saveFile(fileName);

    if (result)
    {
        EditorWindow::prependToRecentFiles(fileName);
    }

    return result;
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
    connect(mOpenAct, &QAction::triggered, this, &EditorWindow::openExistingFile);
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

    QMenu *recentMenu = fileMenu->addMenu(tr("Recent..."));
    connect(recentMenu, &QMenu::aboutToShow, this, &EditorWindow::updateRecentFileActions);
    mRecentFileSubMenuAct = recentMenu->menuAction();

    for (int i = 0; i < MAX_RECENT_FILES; ++i)
    {
        mRecentFileActs[i] = recentMenu->addAction(QString(), this, &EditorWindow::openRecentFile);
        mRecentFileActs[i]->setVisible(false);
    }

    mRecentFileSeparator = fileMenu->addSeparator();

    setRecentFilesVisible(EditorWindow::hasRecentFiles());

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));

    createCommandsEditToolBar();

    connect(mCyclogramWidget, SIGNAL(selectionChanged(ShapeItem*)), this, SLOT(onCyclogramSelectionChanged(ShapeItem*)));

    /*
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &EditorWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
*/
    QMenu *cyclogramMenu = menuBar()->addMenu(tr("Cyclogram"));
    QToolBar *cyclogramToolBar = addToolBar(tr("Cyclogram"));
    cyclogramToolBar->setIconSize(QSize(TOOLBAR_ICON_SIZE, TOOLBAR_ICON_SIZE));

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
    cyclogramMenu->addAction(mRunAct);
    cyclogramToolBar->addAction(mRunAct);

    QIcon stopIcon = QIcon(":/resources/images/stop");
    mStopAct = new QAction(stopIcon, tr("Stop"), this);
    //stopAct->setShortcuts(QKeySequence::New);
    mStopAct->setStatusTip(tr("Stop cyclogram execution"));
    connect(mStopAct, &QAction::triggered, this, &EditorWindow::stopCyclogram);
    cyclogramMenu->addAction(mStopAct);
    cyclogramToolBar->addAction(mStopAct);

    cyclogramMenu->addSeparator();
    cyclogramToolBar->addSeparator();

    QIcon settingsIcon = QIcon(":/resources/images/settings");
    mSettingsAct = new QAction(settingsIcon, tr("Settings"), this);
    mSettingsAct->setStatusTip(tr("Cyclogram settings"));
    connect(mSettingsAct, &QAction::triggered, this, &EditorWindow::showCyclogramSettings);
    cyclogramMenu->addAction(mSettingsAct);
    cyclogramToolBar->addAction(mSettingsAct);

    const QIcon addVariablesIcon = QIcon(":/resources/images/variable");
    mShowVariablesAct = new QAction(addVariablesIcon, tr("Variables"), this);
    mShowVariablesAct->setStatusTip(tr("Show cyclogram variables"));
    connect(mShowVariablesAct, &QAction::triggered, this, &EditorWindow::showVariables);
    cyclogramMenu->addAction(mShowVariablesAct);
    cyclogramToolBar->addAction(mShowVariablesAct);

    const QIcon addMonitorIcon = QIcon(":/resources/images/monitor_auto");
    mAddMonitorAct = new QAction(addMonitorIcon, tr("Add monitor"), this);
    mAddMonitorAct->setStatusTip(tr("Add variables monitor"));
    connect(mAddMonitorAct, &QAction::triggered, this, &EditorWindow::addVariablesMonitor);
    cyclogramMenu->addAction(mAddMonitorAct);
    cyclogramToolBar->addAction(mAddMonitorAct);

//    QMenu *monitorMenu = menuBar()->addMenu(tr("&Monitor"));
//    QToolBar *monitorToolBar = addToolBar(tr("Monitor"));
//    monitorToolBar->setIconSize(QSize(TOOLBAR_ICON_SIZE, TOOLBAR_ICON_SIZE));

//    const QIcon addManualMonitorIcon = QIcon(":/resources/images/monitor_manual");
//    QAction *addManualMonitorAct = new QAction(addManualMonitorIcon, tr("Make data snapshot"), this);
//    addManualMonitorAct->setStatusTip(tr("Make data snapshot"));
//    connect(addManualMonitorAct, &QAction::triggered, this, &EditorWindow::makeDataSnapshot);
//    monitorMenu->addAction(addManualMonitorAct);
//    monitorToolBar->addAction(addManualMonitorAct);

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

    stopCyclogram();
}

void EditorWindow::createCommandsEditToolBar()
{
    //QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    if (mCommandsEditToolbar)
    {
        return;
    }

    mCommandsEditToolbar = new CommandsEditToolbar(this);
    mCommandsEditToolbar->setIconSize(QSize(TOOLBAR_ICON_SIZE, TOOLBAR_ICON_SIZE));
    mCommandsEditToolbar->setAllowedAreas(Qt::LeftToolBarArea);

    addToolBar(Qt::LeftToolBarArea, mCommandsEditToolbar);

    QAction * deleteAction = mCommandsEditToolbar->deleteAction();
    connect(deleteAction, &QAction::triggered, this, &EditorWindow::deleteSelected);

    connect(mCommandsEditToolbar, SIGNAL(currentCommandChanged(int)), mCyclogramWidget, SLOT(setCurrentCommandType(int)));
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

    // open last opened file
    QString fileName = settings.value(LAST_OPENED_FILE_NAME).toString();
    if (!fileName.isEmpty())
    {
        openFile(Cyclogram::defaultStorePath() + fileName);
    }

    auto cyclogram = mCyclogram.lock();
    if (!cyclogram)
    {
        setNewCyclogram(CyclogramManager::createCyclogram());
        setCurrentFile(QString());
    }
}

void EditorWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

bool EditorWindow::maybeSave(int* action)
{
    auto cyclogram = mCyclogram.lock();
    if (!cyclogram || !cyclogram->isModified())
    {
        return true;
    }

    const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("OKB5 Test Studio"),
                               tr("The cyclogram has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (action)
    {
        *action = ret;
    }

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

void EditorWindow::loadFile(const QString &fileName, int options)
{
    closeAll(options);

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
        saveLastOpenedFile(fileName);
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File loaded"), 2000);
    }

    mSaveAct->setDisabled(true);
    mCommandsEditToolbar->reset();
}

void EditorWindow::saveLastOpenedFile(const QString& fileName)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    QStringList tokens = fileName.split(Cyclogram::defaultStorePath());
    if (tokens.size() == 2)
    {
        settings.setValue(LAST_OPENED_FILE_NAME, tokens.at(1));
    }
    else
    {
        settings.setValue(LAST_OPENED_FILE_NAME, QString());
    }
}

bool EditorWindow::saveFile(const QString &fileName)
{
    // check file version
    bool ok;
    Version fileVersion = FileReader::fileVersion(fileName, &ok);
    const Version& appVersion = AppSettings::instance().version();

    if (ok && fileVersion.versionCode() > appVersion.versionCode())
    {
        QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("File version mismatch"),
                                                               tr("Existing file version (%1) is newer than application version (%2).\nSome file data will be lost if you save this file.\nDo you still want to save this file?")
                                                               .arg(fileVersion.toString())
                                                               .arg(appVersion.toString()),
                                                                QMessageBox::Save | /*QMessageBox::Discard |*/ QMessageBox::Cancel);

        if (ret == QMessageBox::Cancel)
        {
            return false;
        }
    }

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
        mNeedHashUpdate = true;

        // save last save dir
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        QString savePath = QFileInfo(fileName).absoluteDir().path();
        settings.setValue(SETTING_LAST_SAVE_FILE_DIR, savePath);
        saveLastOpenedFile(fileName);

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
    cyclogram->setModified(false, false, false);
    setWindowModified(false);

    if (!mFileWatcher->files().empty())
    {
        mFileWatcher->removePaths(mFileWatcher->files());
    }

    QString shownName = mCurFile;
    if (mCurFile.isEmpty())
    {
        mCurFileHash.clear();
        shownName = tr("New file");
    }
    else
    {
        mCurFileHash = FileReader::fileHash(mCurFile);
        mFileWatcher->addPath(fileName);
    }

    shownName += "[*]";
    setWindowFilePath(shownName);
    setWindowTitle(shownName);

    if (!fileName.isEmpty())
    {
        EditorWindow::prependToRecentFiles(fileName);
    }
}

QString EditorWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void EditorWindow::runCyclogram()
{
    mSnapshotsCouner = 0;
    auto cyclogram = mCyclogram.lock();

    mCyclogramWidget->clearSelection();

    Command* errorCmd = cyclogram->validate();
    if (errorCmd)
    {
        mCyclogramWidget->showValidationError(errorCmd);
        LOG_ERROR(QString("Cyclogram validation failed"));
        return;
    }

    if (hasUnsavedChanges() && !trySaveBeforeRun())
    {
        return;
    }

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
    mCommandsEditToolbar->setEnabled(false);
    mRunAct->setEnabled(false);
    mShowVariablesAct->setEnabled(false);
    mSettingsAct->setEnabled(false);
    //mAddMonitorAct->setEnabled(false);
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
    mShowVariablesAct->setEnabled(true);
    mSettingsAct->setEnabled(true);
    //mAddMonitorAct->setEnabled(true);
    mCommandsEditToolbar->setEnabled(true);
}

void EditorWindow::showCyclogramSettings()
{
    CyclogramSettingsDialog dialog(Q_NULLPTR);
    dialog.setCyclogram(mCyclogram.lock());
    dialog.exec();
}

void EditorWindow::showVariables()
{
    VariablesWindow variablesWindow(this);
    variablesWindow.setCyclogram(mCyclogram);
    variablesWindow.exec();
}

void EditorWindow::makeDataSnapshot()
{
    ++mSnapshotsCouner;
    auto cyclogram = mCyclogram.lock();
    cyclogram->variableController()->makeDataSnapshot(QString("Label %1").arg(mSnapshotsCouner));
}

void EditorWindow::addVariablesMonitor()
{
    CyclogramChartDialog* dialog = new CyclogramChartDialog(this);

    dialog->setCyclogram(mCyclogram);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void EditorWindow::onCyclogramFinish(const QString& errorText)
{
    CyclogramEndDialog dialog(this);
    stopCyclogram();

    dialog.setCyclogram(mCyclogram);

    if (!errorText.isEmpty())
    {
        dialog.setText(errorText);
    }
    else
    {
        dialog.setText(tr("Cyclogram execution finished"));
    }

    dialog.exec();
}

void EditorWindow::runModalCyclogram(const QString& shortFileName, const QString& text)
{
    if (shortFileName.isEmpty())
    {
        return;
    }

    ModalCyclogramExecutionDialog dialog(Q_NULLPTR);
    if (dialog.init(shortFileName, text, mSystemState, mCyclogramConsole))
    {
        setEnabled(false);
        dialog.exec();
        setEnabled(true);
    }
}

void EditorWindow::onCyclogramStateChanged(int state)
{
    auto cyclogram = mCyclogram.lock();
    mOpenAct->setEnabled(state == Cyclogram::IDLE);
    mNewAct->setEnabled(state == Cyclogram::IDLE);
    mSaveAct->setEnabled(state == Cyclogram::IDLE && cyclogram->isModified());

#ifdef ENABLE_CYCLOGRAM_PAUSE
    if (state == Cyclogram::PAUSED)
    {
        mRunAct->setIcon(mPlayIcon);
        mRunAct->setStatusTip(tr("Execute cyclogram"));
    }
#endif

    if (state == Cyclogram::IDLE)
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

    cyclogram->setModified(false, false/*true*/, true);
}

void EditorWindow::onSettings()
{
    AppSettingsDialog dialog(this);
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

    connect(mCommandsEditToolbar, SIGNAL(currentCommandChanged(int)), dialog->cyclogramWidget(), SLOT(setCurrentCommandType(int)));
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

void EditorWindow::prependToRecentFiles(const QString &fileName)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList oldRecentFiles = readRecentFiles(settings);
    QStringList recentFiles = oldRecentFiles;
    recentFiles.removeAll(fileName);
    recentFiles.removeAll("");
    recentFiles.prepend(fileName);
    if (oldRecentFiles != recentFiles)
    {
        writeRecentFiles(recentFiles, settings);
    }

    setRecentFilesVisible(!recentFiles.isEmpty());
}

void EditorWindow::setRecentFilesVisible(bool visible)
{
    mRecentFileSubMenuAct->setVisible(visible);
    mRecentFileSeparator->setVisible(visible);
}

bool EditorWindow::hasRecentFiles()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const int count = settings.beginReadArray(recentFilesKey());
    settings.endArray();
    return (count > 0);
}

void EditorWindow::updateRecentFileActions()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList recentFiles = readRecentFiles(settings);
    const int count = qMin(int(MAX_RECENT_FILES), recentFiles.size());

    for (int i = 0; i < MAX_RECENT_FILES; ++i)
    {
        if (i < count)
        {
            QFileInfo info(recentFiles.at(i));
            const QString fileName = info.fileName();
            if (!fileName.isEmpty() && info.exists())
            {
                mRecentFileActs[i]->setText(tr("&%1: %2").arg(i + 1).arg(fileName));
                mRecentFileActs[i]->setData(recentFiles.at(i));
                mRecentFileActs[i]->setVisible(true);
            }
            else
            {
                mRecentFileActs[i]->setVisible(false);
            }
        }
        else
        {
            mRecentFileActs[i]->setVisible(false);
        }
    }
}

void EditorWindow::openRecentFile()
{
    if (const QAction *action = qobject_cast<const QAction *>(sender()))
    {
        openFile(action->data().toString());
    }
}

void EditorWindow::deleteSelected()
{
    mCyclogramWidget->deleteSelectedItem();
}

void EditorWindow::onCyclogramSelectionChanged(ShapeItem* item)
{
    mCommandsEditToolbar->deleteAction()->setEnabled(item != Q_NULLPTR);
}

bool EditorWindow::trySaveBeforeRun()
{
    // save cyclograms before run
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    int saveBeforeStartFlag = settings.value(SILENT_SAVE_BEFORE_START_FLAG, 0).toInt();

    if (saveBeforeStartFlag == 0)
    {
        SaveBeforeRunDialog dialog(this);
        int result = dialog.exec();
        if (result == QDialog::Accepted)
        {
            updateSubprogramDialogs(EditorWindow::Save);
            if (dialog.doNotAskAgain())
            {
                settings.setValue(SILENT_SAVE_BEFORE_START_FLAG, 1);
            }

            if (mSaveAct->isEnabled())
            {
                save();
            }

            return true;
        }

        return false;
    }

    return true;
}

bool EditorWindow::hasUnsavedChanges() const
{
    foreach (QObject* object, mOpenedSubprogramDialogs)
    {
        SubProgramDialog* dialog = qobject_cast<SubProgramDialog*>(object);
        if (dialog && dialog->command()->cyclogram()->isModified())
        {
            return true;
        }
    }

    return mSaveAct->isEnabled();
}

void EditorWindow::reloadCyclogram()
{
    if (mCurFile.isEmpty())
    {
        return;
    }

    if (mNeedHashUpdate)
    {
        mCurFileHash = FileReader::fileHash(mCurFile);
        mNeedHashUpdate = false;
        return;
    }

    QString newHash = FileReader::fileHash(mCurFile);
    if (mCurFileHash == newHash)
    {
        return;
    }

    mCurFileHash = newHash;

    LOG_WARNING(QString("File '%1' is modified outside the editor. Reloading file").arg(mCurFile));
    openFile(mCurFile);
}
