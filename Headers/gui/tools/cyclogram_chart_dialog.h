#ifndef MONITOR_AUTO_H
#define MONITOR_AUTO_H

#include <QDialog>
#include <QMap>

class QCheckBox;
class QTableWidget;
class QCustomPlot;
class Cyclogram;

class CyclogramChartDialog : public QDialog
{
    Q_OBJECT

public:
    CyclogramChartDialog(QWidget * parent);
    ~CyclogramChartDialog();

    void setCyclogram(QSharedPointer<Cyclogram> cyclogram, const QStringList& variables);

private slots:
    //void updateGraphs(const VariableController::DataSnapshot& data); //TODO possibly not needed
    void onVariableValueChanged(const QString& name, qreal value);
    void onCyclogramStateChanged(int state);

private:
    void addRow(int row, const QString& name, qreal value);

    QCustomPlot* mPlot;
    QTableWidget* mVariablesTable;

    QWeakPointer<Cyclogram> mCyclogram;
    qreal mMinY;
    qreal mMaxY;
    qreal mMinX;
    qreal mMaxX;
    qint64 mStartTime;
};

#endif // MONITOR_AUTO_H
