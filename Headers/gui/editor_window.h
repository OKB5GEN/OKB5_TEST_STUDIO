#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QSessionManager;
class QScrollArea;
QT_END_NAMESPACE

class CyclogramWidget;
class Cyclogram;
class VariablesWindow;
class SystemState;

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    EditorWindow();

    void loadFile(const QString &fileName);
    void init();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void about();
    void documentWasModified();

    void runCyclogram();
    void runOneCommand();
    void stopCyclogram();

    void addVariablesMonitor();
    void addManualMonitor();
    void addAutoMonitor();

    void onCyclogramFinish(const QString& errorText);
    void onCyclogramStateChanged(int state);

    void commitData(QSessionManager &);

private:
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    Cyclogram* mCyclogram;
    CyclogramWidget *mCyclogramWidget;

    VariablesWindow* mVariablesWindow;
    SystemState* mSystemState;

    QString mCurFile;

    QAction* mRunAct;
    QAction* mStopAct;
    //QAction* mRunOneCmdAct;

    QIcon mPlayIcon;
    QIcon mPauseIcon;

    QScrollArea * mScrollArea;
    qreal mScaleFactor;
};
#endif // EDITORWINDOW_H
