#ifndef VARIABLES_WINDOW_H
#define VARIABLES_WINDOW_H

#include "Headers/gui/tools/restorable_dialog.h"

class QTableWidget;
class QToolButton;
class QDoubleValidator;
class QCheckBox;
class QLineEdit;
class QTableWidgetItem;

class Cyclogram;

class VariablesWindow: public RestorableDialog
{
    Q_OBJECT

public:
    VariablesWindow(QWidget * parent);
    ~VariablesWindow();

    void setCyclogram(QSharedPointer<Cyclogram> cyclogram);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

    void onSelectAllCheckBoxStateChanged(int state);
    void onItemChanged(QTableWidgetItem* item);
    void onItemDoubleClicked(QTableWidgetItem* item);

    void onAccept();
    void onTableSelectionChanged();

private:
    void addRow(int row, const QString& name, qreal defaultValue, const QString& description, bool existingVariable);
    bool isVariableExist(const QString& name, int excludeRow = -1) const;
    void moveItem(int offset);

    QTableWidget* mTableWidget;
    QToolButton* mAddBtn;
    QToolButton* mRemoveBtn;
    QToolButton* mMoveUpBtn;
    QToolButton* mMoveDownBtn;

    QCheckBox* mSelectAllBox;
    QCheckBox* mShowAllBox;

    QString mPrevText;
    QWeakPointer<Cyclogram> mCyclogram;
};

#endif // VARIABLES_WINDOW_H
