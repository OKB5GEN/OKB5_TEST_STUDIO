#ifndef MONITOR_AUTO_H
#define MONITOR_AUTO_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QCheckBox;
QT_END_NAMESPACE

class QCustomPlot;

class MonitorAuto : public QDialog
{
    Q_OBJECT

public:
    MonitorAuto(QWidget * parent);
    ~MonitorAuto();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();

    void setUpdatePeriod(QString period);

    void getCurrentValue();
    void updateUI();

private:
    QTimer* mTimer;

    QCustomPlot* mPlot;
    QCheckBox* mPlotCheckBox;

    int mUpdatePeriod;
};

#endif // MONITOR_AUTO_H
