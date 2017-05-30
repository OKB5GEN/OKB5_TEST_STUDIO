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
    SubProgramDialog(CmdSubProgram* command, QWidget * parent);
    ~SubProgramDialog();

    CyclogramWidget* cyclogramWidget() const;

private slots:
    void onSaveClick();
    void onVariablesClick();
    void onChartClick();
    void onSettingsClick();

private:
    CmdSubProgram* mCommand;
    CyclogramWidget* mCyclogramWidget;
    VariablesWindow* mVariablesWindow;

    QScrollArea * mScrollArea;
};

#endif // SUBPROGRAM_DIALOG_H
