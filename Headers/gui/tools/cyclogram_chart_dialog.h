#ifndef MONITOR_AUTO_H
#define MONITOR_AUTO_H

#include <QDialog>
#include <QMap>

class QCheckBox;
class QTableWidget;
class QCustomPlot;
class QCPGraph;
class QToolButton;
class Cyclogram;

class CyclogramChartDialog : public QDialog
{
    Q_OBJECT

public:
    CyclogramChartDialog(QWidget * parent);
    ~CyclogramChartDialog();

    void setCyclogram(QSharedPointer<Cyclogram> cyclogram);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onVariableValueChanged(const QString& name, qreal value);
    void onVariableNameChanged(const QString& newName, const QString& oldName);
    void onVariableRemoved(const QString& name);
    void onCyclogramStateChanged(int state);

    void onCellDoubleClicked(int row, int column);
    void onTableSelectionChanged();
    void onShowChartBoxStateChanged(int state);

    void onAddClicked();
    void onRemoveClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

private:
    void addRow(int row, const QString& name, qreal value);
    QCPGraph* variableGraph(const QString& name) const;
    void removeVariableGraph(const QString& name);

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
