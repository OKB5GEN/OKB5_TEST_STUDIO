#ifndef MONITOR_MANUAL_H
#define MONITOR_MANUAL_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE


class MonitorManual : public QDialog
{
    Q_OBJECT

public:
    MonitorManual(QWidget * parent);
    ~MonitorManual();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void addParam();
    void refreshParams();
    void removeParam();

    // TODO remove
    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();

    void setUpdatePeriod(QString period);

    void setAutoMode();
    void setManualMode();

    void getCurrentValue();
    void updateUI();

private:
    QVBoxLayout * mLayout;
    QVector<QObject*> mDeleteButtons;
};

#endif // MONITOR_MANUAL_H
