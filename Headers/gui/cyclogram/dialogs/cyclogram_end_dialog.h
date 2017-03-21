#ifndef CYCLOGRAM_END_DIALOG_H
#define CYCLOGRAM_END_DIALOG_H

#include <QDialog>

class QLabel;
class Cyclogram;

class CyclogramEndDialog: public QDialog
{
    Q_OBJECT

public:
    CyclogramEndDialog(QWidget * parent);
    ~CyclogramEndDialog();

    void setCyclogram(Cyclogram* cyclogram);
    void setText(const QString& text);

private slots:
    void saveReportAs();

private:
     QLabel* mLabel;
     Cyclogram* mCyclogram;
};
#endif // CYCLOGRAM_END_DIALOG_H
