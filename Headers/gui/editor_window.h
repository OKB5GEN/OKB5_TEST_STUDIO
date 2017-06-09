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
class VariablesWindow;
class SystemState;
class CyclogramChartDialog;
class CmdSubProgram;
class SubProgramDialog;

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    EditorWindow();

    void loadFile(const QString &fileName);
    void onApplicationStart();

    SubProgramDialog* subprogramDialog(CmdSubProgram* command) const;
    void addSuprogramDialog(CmdSubProgram* command, SubProgramDialog* dialog);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool save();
    bool saveAs();
    void onSettings();
    void about();
    void documentWasModified();

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
    void runModalCyclogram(const QString& shortFileName, const QString& text);

    void closeAll();
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    void setNewCyclogram(QSharedPointer<Cyclogram> cyclogram);

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

    int mSnapshotsCouner; // hack for force data snapshot save

    QMap<QObject*, QObject*> mOpenedSubprogramDialogs;
};
#endif // EDITOR_WINDOW_H
