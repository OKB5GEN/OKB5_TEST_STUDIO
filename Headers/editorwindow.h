#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
QT_END_NAMESPACE

class RenderArea;
class SortingBox;
class Cyclogram;
class CyclogramEndDialog;

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    EditorWindow();

    void loadFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void about();
    void documentWasModified();

    void runCyclogram();
    void stopCyclogram();
    void addMonitor();

    void addManualMonitor();
    void addAutoMonitor();

    void onCyclogramFinish();

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

    SortingBox *mRenderArea;

    CyclogramEndDialog * mCyclogramEndDialog;

    //QPlainTextEdit *textEdit;
    QString mCurFile;

    QAction* mRunAct;
    QAction* mStopAct;

    QIcon mPlayIcon;
    QIcon mPauseIcon;
};
#endif // EDITORWINDOW_H
