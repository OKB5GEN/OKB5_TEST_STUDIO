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

class SubProgramDialog : public QDialog
{
    Q_OBJECT

public:
    SubProgramDialog(CmdSubProgram* command, QWidget * mainWindow);
    ~SubProgramDialog();

    CyclogramWidget* cyclogramWidget() const;
    CmdSubProgram* command() const;

public slots:
    void onCommandTextChanged(const QString& newText);
    void onParentWindowTitleChanged(const QString& newParentWindowTitle);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    bool onSaveClick();
    void onVariablesClick();
    void onChartClick();
    void onCyclogramSettingsClick();
    void onCommandSettingsClick();

    void updateTitle(const QString& newTitle);

    void onCyclogramModified();
    void onCyclogramSelectionChanged(ShapeItem* item);

private:
    void updateSize();

    CmdSubProgram* mCommand;
    CyclogramWidget* mCyclogramWidget;
    VariablesWindow* mVariablesWindow;

    QScrollArea * mScrollArea;

    QPushButton* mSaveBtn;
    QPushButton* mVariablesBtn;
    QPushButton* mChartBtn;
    QPushButton* mDeleteBtn;
    QPushButton* mCyclogramSettingsBtn;
    QPushButton* mCommandSettingsBtn;

};

#endif // SUBPROGRAM_DIALOG_H
