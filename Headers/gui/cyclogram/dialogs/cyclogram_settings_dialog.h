#ifndef CYCLOGRAM_SETTINGS_DIALOG_H
#define CYCLOGRAM_SETTINGS_DIALOG_H

#include <QDialog>

class QTextEdit;
class QLineEdit;

class Cyclogram;

class CyclogramSettingsDialog : public QDialog
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
