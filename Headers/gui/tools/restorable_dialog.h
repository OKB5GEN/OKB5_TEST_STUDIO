#ifndef RESTORABLE_DIALOG_H
#define RESTORABLE_DIALOG_H

#include <QDialog>

class RestorableDialog: public QDialog
{
    Q_OBJECT

public:
    RestorableDialog(QWidget* parent);
    virtual ~RestorableDialog();

protected:
    void readSettings();

private slots:
    void saveSettings();
};

#endif // RESTORABLE_DIALOG_H
