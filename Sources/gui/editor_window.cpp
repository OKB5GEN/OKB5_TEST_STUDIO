#include <QtWidgets>

#include "Headers/gui/editor_window.h"
#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/gui/tools/monitor_manual.h"
#include "Headers/gui/tools/monitor_auto.h"
#include "Headers/logic/cyclogram.h"

#include "Headers/gui/cyclogram/dialogs/cyclogram_end_dialog.h"
#include "Headers/gui/cyclogram/variables_window.h"
#include "Headers/system/system_state.h"

namespace
{
    static const int TOOLBAR_ICON_SIZE = 64;
}

EditorWindow::EditorWindow():
    mVariablesWindow(Q_NULLPTR),
    mScaleFactor(1.0)
{
    mScrollArea = new QScrollArea(this);

    mCyclogramWidget = new CyclogramWidget(this);
    mCyclogram = new Cyclogram(this);

    mCyclogram->createDefault();
    mCyclogramWidget->load(mCyclogram);

    connect(mCyclogram, SIGNAL(finished(const QString&)), this, SLOT(onCyclogramFinish(const QString&)));
    connect(mCyclogram, SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));

    mScrollArea->setBackgroundRole(QPalette::Dark);
    mScrollArea->setWidget(mCyclogramWidget);
    setCentralWidget(mScrollArea);

    //resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

    createActions();
    createStatusBar();

    readSettings();

    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest, this, &EditorWindow::commitData);

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);

    setWindowTitle(tr("OKB5 Test Studio"));

    mSystemState = new SystemState(this);

    mCyclogram->setSystemState(mSystemState);
}

void EditorWindow::init()
{
    mSystemState->init();
}

void EditorWindow::closeEvent(QCloseEvent *event)
{
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
        int TODO; // create new cyclogram

        //textEdit->clear();
        setCurrentFile(QString());
    }
}

void EditorWindow::open()
{
    if (maybeSave())
    {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
        {
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
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }

    return saveFile(dialog.selectedFiles().first());
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
    bool isModified = true; // TODO check cyclogram modified

    setWindowModified(isModified);
}

void EditorWindow::createActions()
{
    // uncomment to create file menu
    /*
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
    */

    /*
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolBar = addToolBar(tr("Edit"));

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
    QAction *cutAct = new QAction(cutIcon, tr("Cu&t"), this);

    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    //connect(cutAct, &QAction::triggered, textEdit, &QPlainTextEdit::cut);
    editMenu->addAction(cutAct);
    editToolBar->addAction(cutAct);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    QAction *copyAct = new QAction(copyIcon, tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    //connect(copyAct, &QAction::triggered, textEdit, &QPlainTextEdit::copy);
    editMenu->addAction(copyAct);
    editToolBar->addAction(copyAct);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
    QAction *pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    //connect(pasteAct, &QAction::triggered, textEdit, &QPlainTextEdit::paste);
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);

    menuBar()->addSeparator();

#endif // !QT_NO_CLIPBOARD

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &EditorWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

#ifndef QT_NO_CLIPBOARD
    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    //connect(textEdit, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
    //connect(textEdit, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
#endif // !QT_NO_CLIPBOARD
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
    monitorToolBar->addAction(addAutoMonitorAct);
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
    bool isModified = false; // TODO is cyclogram modified

    if (!isModified)
    {
        return true;
    }

    const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
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
        QMessageBox::warning(this, tr("Application"), tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    //QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    //textEdit->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool EditorWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Application"), tr("Cannot write file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }


    //QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    //out << textEdit->toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void EditorWindow::setCurrentFile(const QString &fileName)
{
    mCurFile = fileName;
    //textEdit->document()->setModified(false);
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
        //if (textEdit->document()->isModified())
        {
            save();
        }
    }
}

