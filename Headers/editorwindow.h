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

#define USE_SORTING_BOX


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
    void pauseCyclogram();
    void stopCyclogram();

#ifndef QT_NO_SESSIONMANAGER
    void commitData(QSessionManager &);
#endif

private:
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

#ifdef USE_SORTING_BOX
    SortingBox *renderArea;
#else
    RenderArea *renderArea;
#endif

    //QPlainTextEdit *textEdit;
    QString curFile;
};
#endif // EDITORWINDOW_H
