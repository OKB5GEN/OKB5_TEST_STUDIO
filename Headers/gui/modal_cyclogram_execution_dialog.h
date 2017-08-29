#ifndef MODAL_CYCLOGRAM_EXECUTION_DIALOG_H
#define MODAL_CYCLOGRAM_EXECUTION_DIALOG_H

#include <QDialog>

class SystemState;
class Cyclogram;
class CyclogramConsole;
class QLabel;

class ModalCyclogramExecutionDialog: public QDialog
{
    Q_OBJECT

public:
    ModalCyclogramExecutionDialog(QWidget * parent);
    ~ModalCyclogramExecutionDialog();

    bool init(const QString& fileName, const QString& text, SystemState* systemState, CyclogramConsole* console);

private slots:
    void onCyclogramFinish(const QString& error);

private:
    QWeakPointer<Cyclogram> mCyclogram;
    QLabel* mText;
};
#endif // MODAL_CYCLOGRAM_EXECUTION_DIALOG_H
