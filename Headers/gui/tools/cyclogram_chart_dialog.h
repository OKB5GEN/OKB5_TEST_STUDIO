#ifndef MONITOR_AUTO_H
#define MONITOR_AUTO_H

#include <QDialog>
#include <QMap>

class QCheckBox;
class QTableWidget;
class QCustomPlot;
class QToolButton;
class Cyclogram;

class CyclogramChartDialog : public QDialog
{
    Q_OBJECT

public:
    CyclogramChartDialog(QWidget * parent);
    ~CyclogramChartDialog();

    void setCyclogram(QSharedPointer<Cyclogram> cyclogram);

private slots:
    void onVariableValueChanged(const QString& name, qreal value);
    void onCyclogramStateChanged(int state);

    void onShowVariableChartSelectionChanged(int state);

    void onCellDoubleClicked(int row, int column);
    void onTableSelectionChanged();

    void onAddClicked();
    void onRemoveClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

private:
    void addRow(int row, const QString& name, qreal value);

    QCustomPlot* mPlot;
    QTableWidget* mVariablesTable;

    QToolButton* mAddBtn;
    QToolButton* mRemoveBtn;
    QToolButton* mMoveUpBtn;
    QToolButton* mMoveDownBtn;

    QWeakPointer<Cyclogram> mCyclogram;
    qreal mMinY;
    qreal mMaxY;
    qreal mMinX;
    qreal mMaxX;
    qint64 mStartTime;
};

#endif // MONITOR_AUTO_H
