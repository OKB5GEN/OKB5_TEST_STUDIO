#ifndef MONITORDIALOG_H
#define MONITORDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QCheckBox;
QT_END_NAMESPACE

class QCustomPlot;

class MonitorDialog : public QDialog
{
    Q_OBJECT

public:
    MonitorDialog(QWidget * parent);
    ~MonitorDialog();

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

    void setAutoMode();
    void setManualMode();

    void getCurrentValue();
    void updateUI();

private:
    QTimer* mTimer;

    QCustomPlot* mPlot;
    QCheckBox* mPlotCheckBox;

    QList<QWidget*> mAutoModeWidgets;
    QList<QWidget*> mManualModeWidgets;

    bool mIsAutoMode;
    int mUpdatePeriod;
};

#endif // MONITORDIALOG_H
