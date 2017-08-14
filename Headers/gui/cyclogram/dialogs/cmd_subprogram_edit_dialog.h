#ifndef CMD_SUBPROGRAM_EDIT_DIALOG_H
#define CMD_SUBPROGRAM_EDIT_DIALOG_H

#include <QDialog>
#include <QSharedPointer>

class QLineEdit;
class QCheckBox;
class QTableWidget;
class QTextEdit;
class QLabel;

class CmdSubProgram;
class Cyclogram;
class ConsoleTextWidget;

class CmdSubProgramEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdSubProgramEditDialog(QWidget * parent);
    ~CmdSubProgramEditDialog();

    void setCommand(CmdSubProgram* command, QSharedPointer<Cyclogram> callingCyclogram);

private slots:
    void onAccept();
    void openFile();
    void onInputCheckBoxStateChanged(int state);
    void onOutputCheckBoxStateChanged(int state);
    void onShowExtendedSettings(bool checked);
    void updateUI();

private:
    void setupUI();
    void updateTable(QTableWidget* widget, QCheckBox* changedBox, int state);
    bool eventFilter(QObject *obj, QEvent *event) override;

    CmdSubProgram* mCommand;

    QLineEdit* mFileNameStr;
    QLineEdit* mSubprogramNameStr;

    QTableWidget* mInParams;
    QTableWidget* mOutParams;

    // extended settings
    QTextEdit* mCyclogramDescription;
    QPushButton* mShowExtendedSettings;
    QLabel* mDescriptionHeader;

    ConsoleTextWidget* mConsoleTextWidget;

    QWeakPointer<Cyclogram> mCallingCyclogram;
};

#endif // CMD_SUBPROGRAM_EDIT_DIALOG_H
