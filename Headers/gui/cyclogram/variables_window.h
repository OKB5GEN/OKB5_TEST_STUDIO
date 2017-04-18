#ifndef VARIABLES_WINDOW_H
#define VARIABLES_WINDOW_H

#include <QDialog>

class QTableWidget;
class QToolButton;
class QDoubleValidator;
class Cyclogram;

class VariablesWindow: public QDialog
{
    Q_OBJECT

public:
    VariablesWindow(QWidget * parent);
    ~VariablesWindow();

    void setCyclogram(QSharedPointer<Cyclogram> cyclogram);

protected:

private slots:
    void onAddClicked();
    void onRemoveClicked();

    void onNameChanged();
    void onInitialValueChanged();
    void onDescriptionChanged();
    void onTableSelectionChanged();

    void onCurrentValueChanged(const QString& name, qreal value);

private:
    void addRow(int row, const QString& name, qreal initialValue, qreal currentValue, const QString& description);
    void updateTableSize();

    QTableWidget* mTableWidget;
    QToolButton* mRemoveBtn;

    QWeakPointer<Cyclogram> mCyclogram;

    QDoubleValidator* mValidator;
};

#endif // VARIABLES_WINDOW_H
