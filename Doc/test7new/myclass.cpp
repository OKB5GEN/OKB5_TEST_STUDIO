#include "myclass.h"
#include "mainwindow.h"
#include <QDebug>
#include <windows.h>
#include "comport.h"
MyClass::MyClass(QString s) : name(s)
{

}

void MyClass::doWork()
{
    while(1)
    {
        emit send();
        //Sleep(1000);
        //emit send3();
        //Sleep(500);
        //emit send4();
        Sleep(500);
        emit send5();
        Sleep(500);
    }
}
