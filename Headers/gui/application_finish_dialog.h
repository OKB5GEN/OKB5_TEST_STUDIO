#ifndef APPLICATION_FINISH_DIALOG_H
#define APPLICATION_FINISH_DIALOG_H

#include <QDialog>

class Cyclogram;

class ApplicationFinishDialog: public QDialog
{
    Q_OBJECT

public:
    ApplicationFinishDialog(QWidget * parent);
    ~ApplicationFinishDialog();

    bool init();

private slots:
    void onCyclogramFinish(const QString& error);
private:
    Cyclogram* mCyclogram;
};
#endif // APPLICATION_FINISH_DIALOG_H
