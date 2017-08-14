#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

#include <QMainWindow>
#include <QVariant>

class QAction;
class QMenu;
class QSessionManager;
class QScrollArea;

class CyclogramWidget;
class Cyclogram;
class CyclogramConsole;
class CommandsEditToolbar;
class VariablesWindow;
class SystemState;
class CyclogramChartDialog;
class CmdSubProgram;
class SubProgramDialog;
class ShapeItem;

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    EditorWindow();

    void loadFile(const QString &fileName, int options);
    void onApplicationStart();

    SubProgramDialog* subprogramDialog(CmdSubProgram* command) const;
    void addSuprogramDialog(CmdSubProgram* command, SubProgramDialog* dialog);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openExistingFile();
    void openFile(const QString& name);
    bool save();
    bool saveAs();
    void onSettings();
    void about();
    void documentWasModified();

    void deleteSelected();
    void onCyclogramSelectionChanged(ShapeItem* item);

    void runCyclogram();
    void stopCyclogram();
    void showCyclogramSettings();

    void showVariables();
    void makeDataSnapshot();
    void addVariablesMonitor();

    void onCyclogramFinish(const QString& errorText);
    void onCyclogramStateChanged(int state);
    void commitData(QSessionManager &);

    void onSubprogramDestroyed(QObject* object);
    void onSubprogramDialogDestroyed(QObject* object);

signals:
    void documentSaved(bool saved);

private:
    enum
    {
        MAX_RECENT_FILES = 10
    };

    enum UpdateOption
    {
        Save            = 0x01,
        Close           = 0x02,
        SaveAndClose    = (Save | Close),
    };

    void runModalCyclogram(const QString& shortFileName, const QString& text);

    void closeAll(int options);
    void updateSubprogramDialogs(int updateOption);
    void createActions();
    void createStatusBar();
    void createCommandsEditToolBar();

    void readSettings();
    void writeSettings();
    bool maybeSave(int* action = Q_NULLPTR);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    void setNewCyclogram(QSharedPointer<Cyclogram> cyclogram);

    void saveLastOpenedFile(const QString& fileName);

    // recent files functionality TODO move to separate class (settings dependent? AppSettings?)
    static bool hasRecentFiles();
    void prependToRecentFiles(const QString &fileName);
    void setRecentFilesVisible(bool visible);
    void updateRecentFileActions();
    void openRecentFile();

    bool trySaveBeforeRun();
    bool hasUnsavedChanges() const;

    QWeakPointer<Cyclogram> mCyclogram;
    CyclogramWidget *mCyclogramWidget;

    SystemState* mSystemState;

    QString mCurFile;

    QAction* mRunAct;
    QAction* mStopAct;
    QAction* mSettingsAct;
    QAction* mShowVariablesAct;
    QAction* mAddMonitorAct;

    QAction* mOpenAct;
    QAction* mNewAct;
    QAction* mSaveAct;

#ifdef ENABLE_CYCLOGRAM_PAUSE
    QIcon mPlayIcon;
    QIcon mPauseIcon;
#endif

    QScrollArea * mScrollArea;
    qreal mScaleFactor;

    CyclogramConsole* mCyclogramConsole;

    CommandsEditToolbar* mCommandsEditToolbar;

    int mSnapshotsCouner; // hack for force data snapshot save

    QMap<QObject*, QObject*> mOpenedSubprogramDialogs;

    // recent files
    QAction* mRecentFileActs[MAX_RECENT_FILES];
    QAction* mRecentFileSeparator;
    QAction* mRecentFileSubMenuAct;
};
#endif // EDITOR_WINDOW_H
