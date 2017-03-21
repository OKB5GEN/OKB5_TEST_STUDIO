#ifndef MONITOR_AUTO_H
#define MONITOR_AUTO_H

#include <QDialog>
#include "Headers/logic/variable_controller.h"

class QCheckBox;
class QCustomPlot;
class Cyclogram;

class MonitorAuto : public QDialog
{
    Q_OBJECT

public:
    MonitorAuto(QWidget * parent);
    ~MonitorAuto();

    void setCyclogram(Cyclogram * cyclogram);

protected:
//    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
//    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
//    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
//    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();

    void updateGraphs(const VariableController::DataSnapshot& data);
    void onVariableValueChanged(const QString& name, qreal value);
    void onCyclogramStateChanged(int state);

    void setUpdatePeriod(QString period);

    void getCurrentValue();
    void updateUI();

private:
    QTimer* mTimer;

    QCustomPlot* mPlot;
    QCheckBox* mPlotCheckBox;

    int mUpdatePeriod;

    Cyclogram* mCyclogram;
    qreal mMinY;
    qreal mMaxY;
    qreal mMinX;
    qreal mMaxX;
    qint64 mStartTime;
};

#endif // MONITOR_AUTO_H
