#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

#include <QMainWindow>
#include <QSet>

class QAction;
class QMenu;
class QSessionManager;
class QScrollArea;

class CyclogramWidget;
class Cyclogram;
class VariablesWindow;
class SystemState;
class MonitorAuto;

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    EditorWindow();

    void loadFile(const QString &fileName);
    void onApplicationStart();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool save();
    bool saveAs();
    void about();
    void documentWasModified();

    void runCyclogram();
    void stopCyclogram();

    void addVariablesMonitor();
    void makeDataSnapshot();
    void addChartWidget();

    void onCyclogramFinish(const QString& errorText);
    void onCyclogramStateChanged(int state);

    void onAutoMonitorClosed();

    void commitData(QSessionManager &);

signals:
    void documentSaved(bool saved);

private:
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    QWeakPointer<Cyclogram> mCyclogram;
    CyclogramWidget *mCyclogramWidget;

    VariablesWindow* mVariablesWindow;
    SystemState* mSystemState;

    QString mCurFile;

    QAction* mRunAct;
    QAction* mStopAct;

    QAction* mOpenAct;
    QAction* mNewAct;
    QAction* mSaveAct;

#ifdef ENABLE_CYCLOGRAM_PAUSE
    QIcon mPlayIcon;
    QIcon mPauseIcon;
#endif

    QScrollArea * mScrollArea;
    qreal mScaleFactor;

    QSet<MonitorAuto*> mActiveMonitors;

    int mSnapshotsCouner; // hack for force data snapshot save
};
#endif // EDITOR_WINDOW_H
