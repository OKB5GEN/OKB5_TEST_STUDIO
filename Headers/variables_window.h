#ifndef VARIABLES_WINDOW_H
#define VARIABLES_WINDOW_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QTableWidget;
QT_END_NAMESPACE

class VariableController;

class VariablesWindow: public QDialog
{
    Q_OBJECT

public:
    VariablesWindow(QWidget * parent);
    ~VariablesWindow();

    void setVariableController(VariableController * controller);

protected:

private slots:
    void onAddClicked();
    void onRemoveClicked();

    void onNameChanged();
    void onInitialValueChanged();
    //void onCurrentValueChanged();

    void onValueChanged(const QString& name, qreal value, int container);

private:
    void addRow(int row, const QString& name, qreal initialValue, qreal currentValue);
    void removeRow(int row);

    QTableWidget* mTableWidget;

    VariableController* mController;
};

#endif // VARIABLES_WINDOW_H
