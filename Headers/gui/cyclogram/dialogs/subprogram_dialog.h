#ifndef SUBPROGRAM_DIALOG_H
#define SUBPROGRAM_DIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QTableWidget;
class QScrollArea;
class QPushButton;

class CmdSubProgram;
class CyclogramWidget;
class Cyclogram;
class VariablesWindow;
class ShapeItem;
class Clipboard;

class SubProgramDialog : public QDialog
{
    Q_OBJECT

public:
    SubProgramDialog(CmdSubProgram* command, QSharedPointer<Cyclogram> callingCyclogram, QWidget* mainWindow, QSharedPointer<Clipboard> clipboard);
    ~SubProgramDialog();

    CyclogramWidget* cyclogramWidget() const;
    CmdSubProgram* command() const;

public slots:
    void onCommandTextChanged(const QString& newText);
    void onParentWindowTitleChanged(const QString& newParentWindowTitle);
    void save();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    bool onSaveClick();
    void onVariablesClick();
    void onChartClick();
    void onCyclogramSettingsClick();
    void onSettingsClick();

    void updateTitle(const QString& newTitle);

    void onCyclogramModified();
    void onCyclogramStateChanged(int state);
    void onCyclogramSelectionChanged(ShapeItem* item);
    void reload();

private:
    void updateSize();

    CmdSubProgram* mCommand;
    CyclogramWidget* mCyclogramWidget;

    QScrollArea * mScrollArea;

    QPushButton* mSaveBtn;
    QPushButton* mVariablesBtn;
    QPushButton* mChartBtn;
    QPushButton* mDeleteBtn;
    QPushButton* mSettingsBtn;

    QWeakPointer<Cyclogram> mCallingCyclogram;
};

#endif // SUBPROGRAM_DIALOG_H
