#ifndef MONITOR_AUTO_H
#define MONITOR_AUTO_H

#include <QDialog>
#include <QMap>

class QCheckBox;
class QListWidget;
class QCustomPlot;
class Cyclogram;

class CyclogramChartDialog : public QDialog
{
    Q_OBJECT

public:
    CyclogramChartDialog(QWidget * parent);
    ~CyclogramChartDialog();

    void setCyclogram(QSharedPointer<Cyclogram> cyclogram);

private slots:
    //void updateGraphs(const VariableController::DataSnapshot& data); //TODO possibly not needed
    void onVariableValueChanged(const QString& name, qreal value);
    void onCyclogramStateChanged(int state);
    void onVariableSelectionChanged(bool toggled);

private:
    QCustomPlot* mPlot;
    QListWidget* mVariables;

    QMap<QString, QCheckBox*> mCheckboxes;

    QWeakPointer<Cyclogram> mCyclogram;
    qreal mMinY;
    qreal mMaxY;
    qreal mMinX;
    qreal mMaxX;
    qint64 mStartTime;
};

#endif // MONITOR_AUTO_H
