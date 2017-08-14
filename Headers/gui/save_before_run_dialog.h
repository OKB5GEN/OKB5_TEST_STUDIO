#ifndef SAVE_BEFORE_RUN_DIALOG_H
#define SAVE_BEFORE_RUN_DIALOG_H

#include <QDialog>

class QCheckBox;

class SaveBeforeRunDialog: public QDialog
{
    Q_OBJECT
public:
    SaveBeforeRunDialog(QWidget* parent);

    bool doNotAskAgain() const;

private:
    QCheckBox* mDoNotAskAgainBox;

};
#endif // SAVE_BEFORE_RUN_DIALOG_H
