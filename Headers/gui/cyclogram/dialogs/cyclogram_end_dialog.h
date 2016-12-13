#ifndef CYCLOGRAM_END_DIALOG_H
#define CYCLOGRAM_END_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class CyclogramEndDialog: public QDialog
{
    Q_OBJECT

public:
    CyclogramEndDialog(QWidget * parent);
    ~CyclogramEndDialog();

    void setText(const QString& text);

private:
     QLabel* mLabel;
};
#endif // CYCLOGRAM_END_DIALOG_H
