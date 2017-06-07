#ifndef SUBPROGRAM_DIALOG_H
#define SUBPROGRAM_DIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QTableWidget;
class QScrollArea;

class CmdSubProgram;
class CyclogramWidget;
class Cyclogram;
class VariablesWindow;

class SubProgramDialog : public QDialog
{
    Q_OBJECT

public:
    SubProgramDialog(CmdSubProgram* command, QWidget * mainWindow);
    ~SubProgramDialog();

    CyclogramWidget* cyclogramWidget() const;
    CmdSubProgram* command() const;

private slots:
    void onSaveClick();
    void onVariablesClick();
    void onChartClick();
    void onCyclogramSettingsClick();
    void onCommandSettingsClick();

private:
    void updateSize();

    CmdSubProgram* mCommand;
    CyclogramWidget* mCyclogramWidget;
    VariablesWindow* mVariablesWindow;

    QScrollArea * mScrollArea;
};

#endif // SUBPROGRAM_DIALOG_H
