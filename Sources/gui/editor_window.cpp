#include <QtWidgets>

#include "Headers/gui/editor_window.h"
#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/gui/tools/monitor_manual.h"
#include "Headers/gui/tools/monitor_auto.h"
#include "Headers/logic/cyclogram.h"

#include "Headers/gui/cyclogram/dialogs/cyclogram_end_dialog.h"
#include "Headers/gui/cyclogram/variables_window.h"
#include "Headers/system/system_state.h"

#include "Headers/file_reader.h"
#include "Headers/file_writer.h"

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

    mCyclogramWidget = new CyclogramWidget(this);
    mCyclogram = new Cyclogram(this);
    mCyclogram->setMainCyclogram(true);

    mCyclogram->createDefault();
    mCyclogramWidget->load(mCyclogram);

    connect(mCyclogram, SIGNAL(finished(const QString&)), this, SLOT(onCyclogramFinish(const QString&)));
    connect(mCyclogram, SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));

    mScrollArea->setBackgroundRole(QPalette::Dark);
    mScrollArea->setWidget(mCyclogramWidget);
    setCentralWidget(mScrollArea);

    createActions();
    createStatusBar();

    readSettings();

    connect(mCyclogram, SIGNAL(modified()), this, SLOT(documentWasModified()));

    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest, this, &EditorWindow::commitData);

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);

    setWindowTitle(tr("OKB5 Test Studio[*]"));

    mSystemState = new SystemState(this);

    mCyclogram->setSystemState(mSystemState);
}

void EditorWindow::onApplicationStart()
{
    mSystemState->onApplicationStart();
}

void EditorWindow::closeEvent(QCloseEvent *event)
{
    int TODO; // Закрытие приложения
    /*
     * 1. Если есть активная циклограмма, мы спрашиваем "Циклограмма запущена, стопать бум?"
     * 2. Если нет, то тупо закрываем окно и игнорим ивент
     * 3. Если да, то стопаем циклограмму (та в свою очередь вырубит все модули)
     * 4. По завершении стопа циклограммы мы спрашиваем "Сохранить изменения?" (по идее это перед стартом циклограммы должно спрашиваться или делаться автоматически)
     * 5. После чего делаем обычную магию сохранения и сами закрываем приложение.
    */
    if (maybeSave())
    {
        writeSettings();
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
        mCyclogramWidget->setUpdateOnRemove(false);
        mCyclogram->createDefault();
        mCyclogramWidget->setUpdateOnRemove(true);
        mCyclogramWidget->load(mCyclogram);
        setCurrentFile(QString());

        mCyclogram->setModified(true, true);
    }
}

void EditorWindow::open()
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
    setWindowModified(mCyclogram->isModified());

    if (mCyclogram->isModified())
    {
        emit documentSaved(false);
    }
}

void EditorWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));
    fileToolBar->setIconSize(QSize(TOOLBAR_ICON_SIZE, TOOLBAR_ICON_SIZE));

    QIcon newIcon = QIcon(":/images/new.png");
    QAction *newAct = new QAction(newIcon, tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new cyclogram"));
    connect(newAct, &QAction::triggered, this, &EditorWindow::newFile);
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);

    QIcon openIcon = QIcon(":/images/open.png");
    QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &EditorWindow::open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    QIcon saveIcon = QIcon(":/images/save.png");
    QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &EditorWindow::save);
    connect(this, SIGNAL(documentSaved(bool)), saveAct, SLOT(setDisabled(bool)));
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);

    QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this, &EditorWindow::saveAs);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));

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
    QMenu *runMenu = menuBar()->addMenu(tr("&Run"));
    QToolBar *runToolBar = addToolBar(tr("Run"));
    runToolBar->setIconSize(QSize(TOOLBAR_ICON_SIZE, TOOLBAR_ICON_SIZE));

    mPlayIcon = QIcon(":/images/play.png");
    mPauseIcon = QIcon(":/images/pause.png");

    mRunAct = new QAction(mPlayIcon, tr("Run"), this);
    //mRunAct->setShortcuts(QKeySequence::New);
    mRunAct->setStatusTip(tr("Execute cyclogram"));
    connect(mRunAct, &QAction::triggered, this, &EditorWindow::runCyclogram);
    runMenu->addAction(mRunAct);
    runToolBar->addAction(mRunAct);

    /*
    QIcon runOneCmdIcon = QIcon(":/images/step_forward.png");
    mRunOneCmdAct = new QAction(runOneCmdIcon, tr("Run"), this);
    //mRunOneCmdAct->setShortcuts(QKeySequence::New);
    mRunOneCmdAct->setStatusTip(tr("Run one command"));
    connect(mRunOneCmdAct, &QAction::triggered, this, &EditorWindow::runOneCommand);
    runMenu->addAction(mRunOneCmdAct);
    runToolBar->addAction(mRunOneCmdAct);*/

    QIcon stopIcon = QIcon(":/images/stop.png");
    mStopAct = new QAction(stopIcon, tr("Stop"), this);
    //stopAct->setShortcuts(QKeySequence::New);
    mStopAct->setStatusTip(tr("Stop cyclogram execution"));
    connect(mStopAct, &QAction::triggered, this, &EditorWindow::stopCyclogram);
    runMenu->addAction(mStopAct);
    runToolBar->addAction(mStopAct);

    stopCyclogram();

    QMenu *monitorMenu = menuBar()->addMenu(tr("&Monitor"));
    QToolBar *monitorToolBar = addToolBar(tr("Monitor"));
    monitorToolBar->setIconSize(QSize(TOOLBAR_ICON_SIZE, TOOLBAR_ICON_SIZE));

    const QIcon addVariablesIcon = QIcon(":/images/variable.png");
    QAction *addVariablesAct = new QAction(addVariablesIcon, tr("Add variable monitor"), this);
    addVariablesAct->setStatusTip(tr("Add variables monitor"));
    connect(addVariablesAct, &QAction::triggered, this, &EditorWindow::addVariablesMonitor);
    monitorMenu->addAction(addVariablesAct);
    monitorToolBar->addAction(addVariablesAct);
/* // TODO manual and automatic monitor creation commented
    const QIcon addManualMonitorIcon = QIcon(":/images/monitor_manual.png");
    QAction *addManualMonitorAct = new QAction(addManualMonitorIcon, tr("Add manual monitor"), this);
    addManualMonitorAct->setStatusTip(tr("Add manual parameter monitor"));
    connect(addManualMonitorAct, &QAction::triggered, this, &EditorWindow::addManualMonitor);
    monitorMenu->addAction(addManualMonitorAct);
    monitorToolBar->addAction(addManualMonitorAct);

    const QIcon addAutoMonitorIcon = QIcon(":/images/monitor_auto.png");
    QAction *addAutoMonitorAct = new QAction(addAutoMonitorIcon, tr("Add auto monitor"), this);
    addAutoMonitorAct->setStatusTip(tr("Add auto parameter monitor"));
    connect(addAutoMonitorAct, &QAction::triggered, this, &EditorWindow::addAutoMonitor);
    monitorMenu->addAction(addAutoMonitorAct);
    monitorToolBar->addAction(addAutoMonitorAct);*/
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
    if (!mCyclogram->isModified())
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
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("OKB5 Test Studio"),
                                   tr("Cannot read file %1:\n%2.").
                                   arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    mCyclogram->clear();
    mCyclogramWidget->clear();

    FileReader reader(mCyclogram);
    if (!reader.read(&file))
    {
        QMessageBox::warning(this, tr("OKB5 Test Studio"),
                                   tr("Parse error in file %1:\n%2.").
                                   arg(QDir::toNativeSeparators(fileName), reader.errorString()));

        mCyclogram->createDefault();
    }
    else
    {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File loaded"), 2000);
    }

    mCyclogramWidget->load(mCyclogram);
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

    FileWriter writer(mCyclogram);
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
    mCyclogram->setModified(false, false);
    setWindowModified(false);

    QString shownName = mCurFile;
    if (mCurFile.isEmpty())
    {
        shownName = "untitled.txt";
    }

    setWindowFilePath(shownName);
}

QString EditorWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void EditorWindow::runCyclogram()
{
    Command* errorCmd = mCyclogram->validate();
    if (errorCmd)
    {
        mCyclogramWidget->showValidationError(errorCmd);
        qDebug("Cyclogram validation failed");
        return;
    }

    if (mCyclogram->state() == Cyclogram::STOPPED)
    {
        //mCyclogram->setExecuteOneCmd(false);
        //mRunOneCmdAct->setEnabled(false);
        mRunAct->setIcon(mPauseIcon);
        mRunAct->setStatusTip(tr("Pause cyclogram execution"));
        mCyclogram->run();
    }
    else if (mCyclogram->state() == Cyclogram::RUNNING)
    {
        //mRunOneCmdAct->setEnabled(true);
        mRunAct->setIcon(mPlayIcon);
        mRunAct->setStatusTip(tr("Execute cyclogram"));
        mCyclogram->pause();
    }
    else if (mCyclogram->state() == Cyclogram::PAUSED)
    {
        //mRunOneCmdAct->setEnabled(false);
        mRunAct->setIcon(mPauseIcon);
        mRunAct->setStatusTip(tr("Pause cyclogram execution"));
        mCyclogram->resume();
    }

    mStopAct->setEnabled(true);
}

void EditorWindow::runOneCommand()
{
    /*
    Command* errorCmd = mCyclogram->validate();
    if (errorCmd)
    {
        mCyclogramWidget->showValidationError(errorCmd);
        qDebug("Cyclogram validation failed");
        return;
    }

    if (mCyclogram->state() == Cyclogram::STOPPED)
    {
        mCyclogram->setExecuteOneCmd(true);
        mRunOneCmdAct->setEnabled(false);
        mRunAct->setIcon(mPauseIcon);
        mRunAct->setStatusTip(tr("Pause cyclogram execution"));
        mCyclogram->run();
    }
    else if (mCyclogram->state() == Cyclogram::PAUSED)
    {
        mCyclogram->setExecuteOneCmd(true);
        mRunOneCmdAct->setEnabled(false);
        mRunAct->setIcon(mPauseIcon);
        mRunAct->setStatusTip(tr("Pause cyclogram execution"));
        mCyclogram->resume();
    }

    mStopAct->setEnabled(true);
    */
}

void EditorWindow::stopCyclogram()
{
    //mCyclogram->setExecuteOneCmd(false);

    mCyclogram->stop();

    mRunAct->setIcon(mPlayIcon);
    mRunAct->setStatusTip(tr("Execute cyclogram"));
    mStopAct->setEnabled(false);

    //mRunOneCmdAct->setEnabled(true);
    mRunAct->setEnabled(true);
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

void EditorWindow::addManualMonitor()
{
    MonitorManual* dialog = new MonitorManual(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void EditorWindow::addAutoMonitor()
{
    MonitorAuto* dialog = new MonitorAuto(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void EditorWindow::onCyclogramFinish(const QString& errorText)
{
    CyclogramEndDialog * dialog = new CyclogramEndDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    stopCyclogram();

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

void EditorWindow::onCyclogramStateChanged(int state)
{
    if (state == Cyclogram::PAUSED)
    {
        mRunAct->setIcon(mPlayIcon);
        mRunAct->setStatusTip(tr("Execute cyclogram"));
        //mRunOneCmdAct->setEnabled(true);
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
        // Non-interactive: save without asking
        if (mCyclogram->isModified())
        {
            save();
        }
    }
}

