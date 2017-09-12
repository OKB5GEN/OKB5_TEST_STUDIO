#ifndef CMD_DELAY_EDIT_DIALOG_H
#define CMD_DELAY_EDIT_DIALOG_H

#include "Headers/gui/tools/restorable_dialog.h"

class QGridLayout;
class QSpinBox;
class QListWidget;

class CmdDelay;
class ConsoleTextWidget;

class CmdDelayEditDialog : public RestorableDialog
{
    Q_OBJECT

public:
    CmdDelayEditDialog(QWidget * parent);
    ~CmdDelayEditDialog();

    void setCommand(CmdDelay* command);

private slots:
    void onHoursChanged(int hours);
    void onMinutesChanged(int minutes);
    void onSecondsChanged(int seconds);
    void onMilliSecondsChanged(int mseconds);

    void onAccept();

private:
    QSpinBox* addItem(QGridLayout* layout, const QString& text, int row, int max, const char* onChange);

    CmdDelay* mCommand;
    int mHours;
    int mMinutes;
    int mSeconds;
    int mMSeconds;

    QSpinBox* mHSpin;
    QSpinBox* mMSpin;
    QSpinBox* mSSpin;
    QSpinBox* mMSSpin;

    QListWidget* mVariablesList;

    ConsoleTextWidget* mConsoleTextWidget;
};

#endif // CMD_DELAY_EDIT_DIALOG_H
