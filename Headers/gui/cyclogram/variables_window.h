#ifndef VARIABLES_WINDOW_H
#define VARIABLES_WINDOW_H

#include <QDialog>
#include <QSet>
#include <QList>
#include <QPair>

class QTableWidget;
class QToolButton;
class QDoubleValidator;
class QCheckBox;
class QLineEdit;

class Cyclogram;

class VariablesWindow: public QDialog
{
    Q_OBJECT

public:
    VariablesWindow(QWidget * parent);
    ~VariablesWindow();

    void setCyclogram(QSharedPointer<Cyclogram> cyclogram);

private slots:
    void onAddClicked();
    void onRemoveClicked();

    void onNameChanged();
    void updateSize();

    void onSelectAllCheckBoxStateChanged(int state);
    void onShowAllCheckBoxStateChanged(int state);
    void onSelectVarCheckBoxStateChanged(int state);
    void onAccept();

private:
    void addRow(int row, const QString& name, qreal defaultValue, const QString& description);
    bool isVariableExist(const QString& name, QLineEdit* excludeRow) const;
    void optimizeRenameLog();


    QTableWidget* mTableWidget;
    QToolButton* mRemoveBtn;

    QCheckBox* mSelectAllBox;
    QCheckBox* mShowAllBox;

    QWeakPointer<Cyclogram> mCyclogram;

    QDoubleValidator* mValidator;

    QSet<int> mSelectedRows;

    QList<QPair<QString, QString>> mRenameLog;
};

#endif // VARIABLES_WINDOW_H
