#ifndef CYCLOGRAM_SETTINGS_DIALOG_H
#define CYCLOGRAM_SETTINGS_DIALOG_H

#include "Headers/gui/tools/restorable_dialog.h"

class QTextEdit;
class QLineEdit;

class Cyclogram;

class CyclogramSettingsDialog : public RestorableDialog
{
    Q_OBJECT

public:
    CyclogramSettingsDialog(QWidget * parent);
    ~CyclogramSettingsDialog();

    void setCyclogram(QSharedPointer<Cyclogram> cyclogram);

protected:

private slots:
    void onAccept();
    void openFile();

private:
    QLineEdit* mDefaultNameStr;
    QLineEdit* mFileNameStr;

    QTextEdit* mCyclogramDescription;
    QWeakPointer<Cyclogram> mCyclogram;
};

#endif // CYCLOGRAM_SETTINGS_DIALOG_H
