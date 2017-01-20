#include "bp.h"
#include "mainwindow.h"
#include <QDebug>
#include <windows.h>
#include "comport.h"
#include <QTimer>
#include <QTime>

QTimer *MyTimerBP;

bp::bp(QString s) : name(s)
{

}


void bp::bp_avt(int x, int y)
{
    if(x==1) {
        MyTimerBP = new QTimer;
        MyTimerBP->start(y);
        QObject::connect(MyTimerBP,SIGNAL(timeout()), this, SLOT(bp_timer()));
    }
    else
    {
        y=0;
        QObject::disconnect(MyTimerBP,SIGNAL(timeout()), this, SLOT(bp_timer()));
    }
}
void bp::bp_timer()
{
    emit paint();
}
