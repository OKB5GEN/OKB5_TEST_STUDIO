#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "WDMTMKv2.cpp"
#include <QtSerialPort/QtSerialPort>
#include "myclass.h"
#include "bp.h"
#include "OTD.h"
#include "MKO.h"
#include "Tech.h"
#include "math.h"
#include "comport.h"

int cErrors, fEventResult, strt, count_1=0,count_2=0, flag_mko_test=0;
int flag_rem1 = 0, flag_rem2 = 0, flag_con1 = 0, flag_con2 = 0, k, flag_con3 = 0, flag_con4 = 0, flag_con5 = 0, flag_tech_log=0,flag_rm=0;
int flag_mko_auto = 0, flag_mko_osn = 0, flag_mko_rez = 0, flag_otd_auto = 0, flag_bp_auto = 0, flag_tech_auto = 0, flag_tech_ssi=0,flag_rm_4=0;
int flag_rlm=0, flag_rlm2=0;
double dat [1000] = { 0 }, dat1 [1000] = { 0 };
unsigned dwMRT;
int er;
double d_h[3],d_v[3];
uint16_t sum;
WORD tx_data [4];
QString TestOutStr, TestOutStr1, result_tech, error_m;
QString error_mes_1, error_mes_2,error_mes_3,error_mes_4,error_mes_5,error_mes_6,error_mes_7,error_mes_8,error_mes_9;
QString ncd, zcd, cdh, cdd;
QString buf_SSI[20], buf_RM_2[20], buf_V[20]={0}, buf_H[20]={0},delta_V[20]={0}, delta_H[20]={0}, delta_RM_2[20]={0};

QThread *threadTech = new QThread;
Tech *myTech = new Tech ("B");

QThread *threadOTD = new QThread;
OTD *myOTD = new OTD ("B");

QThread *threadMKO = new QThread;
MKO *myMKO = new MKO ("B");

QThread *threadBP = new QThread;
bp *myBP = new bp ("B");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow (parent),
    ui (new Ui::MainWindow)
{
    ui->setupUi (this);
    startcondition ();
}

MainWindow::~MainWindow()
{
    //com5OFF ();
    //com6OFF ();
    COMClose5_6 ();
    COMClose4 ();
    //COMClose8 ();
    delete ui;
}
void MainWindow::startcondition ()
{
    COMConnector5_6 ();
    COMConnector4 ();
    //COMConnector8 ();
    ui->radioButton->setChecked(true);
    ui->radioButton_3->setChecked(true);
    ui->bp_cyc->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->MKO_avt->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->OTD_avt_2->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->pushButton_start_com5->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->pushButton_start_com6->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->pushButton_4->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->pushButton_5->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->pushButton_7->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->pushButton_8->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->pushButton_10->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->pushButton_log->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->tech_avto->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->MKO_osn->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->MKO_rez->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("9600"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("19200"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("38400"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("57600"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("78125"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("115200"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("230400"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("460800"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("468750"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("625000"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("921600"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("1250000"));
    ui->baudRateBox_tech->addItem (QStringLiteral ("3000000"));
    ui->lineEdit_Addr_2->setText (QString::number (0));
    ui->lineEdit_Addr_3->setText (QString::number (0));
    ui->lineEdit_period->setText (QString::number (1000));
    ui->tech_period->setText (QString::number (1000));
    ui->lineEdit_period_OTD->setText (QString::number (1));
    ui->MKO_cd_0->setText (QString::number (0));
    ui->MKO_cd_1->setText (QString::number (0));
    ui->MKO_cd_3->setText (QString::number (0));
    ui->MKO_cd_4->setText (QString::number (0));
    ui->MKO_cd_5->setText (QString::number (0));
    ui->MKO_cd_6->setText (QString::number (0));
    ui->MKO_cd_8->setText (QString::number (0));
    ui->MKO_cd_9->setText (QString::number (0));
    ui->lineEdit_bp_max->setText (QString::number (0.2));
    ui->lineEdit_bp_min->setText (QString::number (-0.01));
    ui->lineEdit_bp_l1->setText (QString::number (1000));
    ui->lineEdit_bp_l2->setText (QString::number (0));
    ui->lineEdit_period_bp->setText (QString::number (1000));
    ui->tableWidget->setColumnCount (7);
    ui->tableWidget->setColumnWidth (0, 85);
    ui->tableWidget->setColumnWidth (1, 85);
    ui->tableWidget->setColumnWidth (2, 85);
    ui->tableWidget->setColumnWidth (3, 85);
    ui->tableWidget->setColumnWidth (4, 85);
    ui->tableWidget->setColumnWidth (5, 85);
    ui->tableWidget->setColumnWidth (6, 85);
    ui->tableWidget->setHorizontalHeaderItem (0, new QTableWidgetItem (tr ("RM44SC")));
    ui->tableWidget->setHorizontalHeaderItem (1, new QTableWidgetItem (tr ("RM44SC_2")));
    ui->tableWidget->setHorizontalHeaderItem (2, new QTableWidgetItem (tr ("Дельта RM")));
    ui->tableWidget->setHorizontalHeaderItem (3, new QTableWidgetItem (tr ("СКВТ Верт.")));
    ui->tableWidget->setHorizontalHeaderItem (4, new QTableWidgetItem (tr ("СКВТ Гориз.")));
    ui->tableWidget->setHorizontalHeaderItem (5, new QTableWidgetItem (tr ("Дельта Верт.")));
    ui->tableWidget->setHorizontalHeaderItem (6, new QTableWidgetItem (tr ("Дельта Гориз.")));
    ui->tableWidget->setShowGrid (true);
    ui->tableWidget->setSelectionMode (QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior (QAbstractItemView::SelectRows);
    ui->tableWidget_2->setColumnCount (4);
    ui->tableWidget_2->setColumnWidth (0, 90);
    ui->tableWidget_2->setColumnWidth (1, 200);
    ui->tableWidget_2->setColumnWidth (2, 90);
    ui->tableWidget_2->setColumnWidth (3, 90);
    ui->tableWidget_2->setHorizontalHeaderItem (0, new QTableWidgetItem (tr ("Номер СД")));
    ui->tableWidget_2->setHorizontalHeaderItem (1, new QTableWidgetItem (tr ("Значение разрядов СД")));
    ui->tableWidget_2->setHorizontalHeaderItem (2, new QTableWidgetItem (tr ("Значение hex")));
    ui->tableWidget_2->setHorizontalHeaderItem (3, new QTableWidgetItem (tr ("Значение dec")));
    ui->tableWidget_2->setShowGrid (true);
    ui->tableWidget_2->setSelectionMode (QAbstractItemView::SingleSelection);
    ui->tableWidget_2->setSelectionBehavior (QAbstractItemView::SelectRows);
    stm_on_mko (1, 0);
    stm_on_mko (2, 0);

    myBP->moveToThread (threadBP);
    connect (myBP, SIGNAL (paint ()), this, SLOT (paintvaluebp ()));
    threadBP->start ();

    myMKO->moveToThread (threadMKO);
    connect (myMKO, SIGNAL (test_MKO (int)), this, SLOT (simpltst1 (int)));
    connect (myMKO, SIGNAL (MKO_CTM (int,int)), this, SLOT (MKO_change_ch (int,int)));
    connect (myMKO, SIGNAL (start_MKO (QString)), this, SLOT (MKO_data (QString)));
    connect (myMKO, SIGNAL (data_MKO (QString)), this, SLOT (MKO_cm_data (QString)));
    threadMKO->start ();

    myTech->moveToThread (threadTech);
    connect (threadTech, SIGNAL (started ()), myTech, SLOT (COMConnectorTech ()));
    connect (myTech, SIGNAL (tech_SSI_value(double, double, double, double)), this, SLOT (Tech_SSI_value (double, double, double, double)));
    connect (myTech, SIGNAL (tech_err(double)), this, SLOT (Tech_error(double)));
    connect (myTech, SIGNAL (tech_buf(QString)), this, SLOT (statusCAN(QString)));
    connect (myTech, SIGNAL (tech_buf1(QString)), this, SLOT (statusRS(QString)));
    threadTech->start ();

    myOTD->moveToThread (threadOTD);
    connect (myOTD, SIGNAL (start_OTDPT (double,double)), this, SLOT (OTDPTdata (double,double)));
    connect (myOTD, SIGNAL (temp_OTD (QString)), this, SLOT (OTDtemd (QString)));
    connect (myOTD, SIGNAL (OTD_res (int)), this, SLOT (OTD_res_st (int)));
    connect (myOTD, SIGNAL (OTD_reqr (QString)), this, SLOT (status_OTD (QString)));
    connect (myOTD, SIGNAL (OTD_err_res (int)), this, SLOT (OTD_err_res (int)));
    connect (myOTD, SIGNAL (OTD_id1 ()), this, SLOT (OTD_id ()));
    connect (myOTD, SIGNAL (OTD_vfw (double)), this, SLOT (OTD_fw (double)));
    connect (myOTD, SIGNAL (err_OTD (QString)), this, SLOT (OTDerror (QString)));
    connect (myOTD, SIGNAL (tm_OTD1 (QString)), this, SLOT (OTDtm1 (QString)));
    connect (myOTD, SIGNAL (tm_OTD2 (QString)), this, SLOT (OTDtm2 (QString)));
    connect (myOTD, SIGNAL (tm_OTD_err ()), this, SLOT (OTDtm_err ()));
    connect (myOTD, SIGNAL (echo(int)), this, SLOT (OTD_ind_echo(int)));
    connect (threadOTD, SIGNAL (started ()), myOTD, SLOT (COMConnectorOTD ()));
    threadOTD->start ();

    QThread *thread = new QThread;
    MyClass *my = new MyClass ("B");
    my->moveToThread (thread);
    connect(my, SIGNAL(send()), this, SLOT(paintvalue()));
    //connect (my, SIGNAL (send3 ()), this, SLOT (statusRS ()));
    //connect (my, SIGNAL (send4 ()), this, SLOT (statusCAN ()));
    connect (my, SIGNAL (send5 ()), this, SLOT (statusM ()));
    connect (thread, SIGNAL (started ()), my, SLOT (doWork ()));
    thread->start ();
    startpower ();

    ui->setU1->setText (QString::number (27));
    ui->setU2->setText (QString::number (27));
    ui->setlimU1->setText (QString::number (28));
    ui->setlimU2->setText (QString::number (28));
    ui->setlimI1->setText (QString::number (2));
    ui->setlimI2->setText (QString::number (2));
    ui->error_mod->setStyleSheet ("font: 25 16pt GOST type A;" "color: red;");
    if (id_stm () != 1){
        error_m += "СТМ: Модуль установлен не в свой слот!\n";
        //ui->error_mod->setText (error_m);

    }
    else error_m ="";
    write_message();
    //if (id_tech () != 1) error_m += " Модуль ТЕХНОЛОГИЧЕСКИЙ установлен не в свой слот!";

    error_m = "";
}

int MainWindow::simpltst1 (int z)
{
    if (z == 0) {
        ui->error_mod->setStyleSheet ("font: 25 10pt GOST type A;" "color: black;");
        TestOutStr = "Тест пройден успешно!\n";
        //ui->error_mod->setText (TestOutStr);
        TestOutStr = "";
    }
    else{
        TestOutStr1 = "";
        ui->error_mod->setStyleSheet ("font: 25 10pt GOST type A;" "color: red;");
        TestOutStr1 += "Ошибка! Тест провален с ";
        TestOutStr1 += QString::number (z);
        TestOutStr1 += " ошибками. Перезагрузите программу!\n";
        //ui->error_mod->setText (TestOutStr1);

    }
    write_message();
    return z;
}

void MainWindow::delete_table_MKO ()
{
    int n = ui->tableWidget_2->rowCount ();

    for (int i = 0; i < n; i++) ui->tableWidget_2->removeRow (0);
}
void MainWindow::delete_table_tech ()
{
    int n = ui->tableWidget->rowCount ();

    for (int i = 0; i < n; i++) ui->tableWidget->removeRow (0);
}
void MainWindow::add_string_table_tech (int n, QString data_rm, QString data_rm_2, QString delta_rm, QString data_v, QString data_h, QString delta_v,QString delta_h)
{
    ui->tableWidget->insertRow (n);
    ui->tableWidget->setItem (n, 0, new QTableWidgetItem (data_rm));
    ui->tableWidget->setItem (n, 1, new QTableWidgetItem (data_rm_2));
    ui->tableWidget->setItem (n, 2, new QTableWidgetItem (delta_rm));
    ui->tableWidget->setItem (n, 3, new QTableWidgetItem (data_v));
    ui->tableWidget->setItem (n, 4, new QTableWidgetItem (data_h));
    ui->tableWidget->setItem (n, 5, new QTableWidgetItem (delta_v));
    ui->tableWidget->setItem (n, 6, new QTableWidgetItem (delta_h));
    ui->tableWidget->setRowHeight (n, 20);

}
int MainWindow::add_string_table_MKO (int n, QString text_data, QString comm_data, QString data_hex, QString data_dec)
{
    ui->tableWidget_2->insertRow (n);
    ui->tableWidget_2->setItem (n, 0, new QTableWidgetItem (text_data));
    ui->tableWidget_2->setItem (n, 1, new QTableWidgetItem (comm_data));
    ui->tableWidget_2->setItem (n, 2, new QTableWidgetItem (data_hex));
    ui->tableWidget_2->setItem (n, 3, new QTableWidgetItem (data_dec));
    ui->tableWidget_2->setRowHeight (n, 35);
    return 1;
}

void MainWindow::MKO_data (QString data)
{
    error_mes_1=data;
    write_message();
    if(error_mes_1==""&& flag_mko_test==1){
        flag_mko_test=0;
        ui->MKO_test->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (flag_mko_test==1){
        flag_mko_test=0;
        ui->MKO_test->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
    }
}
void MainWindow::MKO_cm_data (QString data)
{
    QStringList list1 = data.split (" ");
    QString list2 [42];

    list2 [0] += "Текущий режим работы ось ψ\n(от -32767 до +32767)";
    list2 [1] += "Заданное положение в шагах ось ψ\n( от -214748648 до +214748648)";
    list2 [2] += "Заданное положение в шагах ось ψ\n( от -214748648 до +214748648)";
    list2 [3] += "Текущая скорость ось ψ\n ( от -32767 до +32767)";
    list2 [4] += "Текущий ток ось ψ\n ( от -511 до +511)";
    list2 [5] += "Текущий режим работы ось υ\n(от -32767 до +32767)";
    list2 [6] += "Заданное положение в шагах ось υ\n( от -214748648 до +214748648) ";
    list2 [7] += "Заданное положение в шагах ось υ\n( от -214748648 до +214748648)";
    list2 [8] += "Текущая скорость ось υ\n ( от -32767 до +32767)";
    list2 [9] += "Текущий ток ось υ\n ( от -511 до +511)";
    list2 [10] += "Информация датчика угла по ψ\n (0…180град)";
    list2 [11] += "Информация датчика угла по υ\n (0…180град) ";
    list2 [12] += "Не используется";
    list2 [13] += "Не используется";
    list2 [14] += "Не используется";
    list2 [15] += "Не используется";
    list2 [16] += "Не используется";
    list2 [17] += "Не используется";
    list2 [18] += "Признак ошибки контрольной\n суммы принятого массива";
    list2 [19] += "Информация термодатчика в град.\n (0xFFFF-датчика нет)";
    list2 [20] += "Контрольная сумма";
    delete_table_MKO ();
    ncd = "";
    zcd = "";
    cdh = "";
    cdd = "";
    int ct = 0;
    signed short a;
    if (list1 [0] != "") {
        for (int i = 0; i < 21; i++) {
            int z = list1 [i].toInt ();
            ncd += "CД ";
            ncd += QString::number (i);
            zcd = list2 [i];
            cdh += "0x";
            if (i < 10 && i != 1 && i != 6) {
                a = z;
                cdh += QString::number (z, 16);
                cdd += QString::number (a);
            }
            if (i == 10 || i == 11) {
                int d1 = z * 180.0 / 65536.0;
                double z4=(z * 180.0 / 65536.0-d1)*60.0;
                int m1 =z4;
                int s1 = (z4-m1)*60.0;
                QString V;
                V += QString::number (d1);
                V += "° ";
                V += QString::number (m1);
                V += "' ";
                V += QString::number (s1);
                V += "''";
                //float k = z * 180.0 / 65536.0;
                cdh += QString::number (z, 16);
                cdd += V;
            }
            if (i == 12) {
                i = i + 5;
                ncd += " - СД " + QString::number (i);
                cdh += QString::number (z, 16);
                cdd += QString::number (z);
            }
            if (i == 18 || i == 20) {
                cdh += QString::number (z, 16);
                cdd += QString::number (z);
            }
            if (i == 19) {
                WORD y = z;
                y = y << 3;
                y = y >> 15;
                if (y == 1) {
                    y = z;
                    y = y << 4;
                    y = y >> 4;
                    z = y;
                }
                else z = 0xFFFF;
                cdh += QString::number (z, 16);
                cdd += QString::number (z);
            }
            if (i == 1 || i == 6) {
                ncd += " - СД " + QString::number (i + 1);
                int f = list1 [i].toInt () << 16;
                signed int k1 = f + list1 [i + 1].toInt ();
                z = f + list1 [i + 1].toInt ();
                i++;
                cdh += QString::number (z, 16);
                cdd += QString::number (k1);
            }
            add_string_table_MKO (ct, ncd, zcd, cdh, cdd);
            ct++;
            ncd = "";
            zcd = "";
            cdh = "";
            cdd = "";
        }
    }
    if (flag_mko_osn + flag_mko_rez == 3) {
        if (list1 [0] != "") {
            add_string_table_MKO (ct, "----------- ", "'РЕЗЕРВНЫЙ ПОЛУКОМПЛЕКТ'", "---------- ", "----------- ");
            ct++;
            for (int i = 0; i < 21; i++) {
                int z = list1 [i + 21].toInt ();
                ncd += "CД ";
                ncd += QString::number (i);
                zcd = list2 [i];
                cdh += "0x";
                if (i < 10 && i != 1 && i != 6) {
                    a = z;
                    cdh += QString::number (z, 16);
                    cdd += QString::number (a);
                }
                if (i == 10 || i == 11) {
                    z = z * 180 / 65536;
                    cdh += QString::number (z, 16);
                    cdd += QString::number (z);
                }
                if (i == 12) {
                    i = i + 5;
                    ncd += " - СД " + QString::number (i);
                    cdh += QString::number (z, 16);
                    cdd += QString::number (z);
                }
                if (i == 18 || i == 20) {
                    cdh += QString::number (z, 16);
                    cdd += QString::number (z);
                }
                if (i == 19) {
                    WORD y = z;
                    y = y << 3;
                    y = y >> 15;
                    if (y == 1) {
                        y = z;
                        y = y << 4;
                        y = y >> 4;
                        z = y;
                    }
                    else z = 0xFFFF;
                    cdh += QString::number (z, 16);
                    cdd += QString::number (z);
                }
                if (i == 1 || i == 6) {
                    ncd += " - СД " + QString::number (i + 1);
                    int f = list1 [i + 21].toInt () << 16;
                    signed int k1 = f + list1 [i + 22].toInt ();
                    z = f + list1 [i + 22].toInt ();
                    i++;
                    cdh += QString::number (z, 16);
                    cdd += QString::number (k1);
                }
                add_string_table_MKO (ct, ncd, zcd, cdh, cdd);
                ct++;
                ncd = "";
                zcd = "";
                cdh = "";
                cdd = "";
            }
        }
    }
}

void MainWindow::on_pushButton_start_com6_clicked ()
{
    if (flag_rem1 == 0) {
        flag_rem1 = 1;
        com6ON ();
        ui->pushButton_start_com6->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else{
        flag_rem1 = 0;
        com6OFF ();
        ui->pushButton_start_com6->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
}

void MainWindow::on_pushButton_start_com5_clicked ()
{
    if (flag_rem2 == 0) {
        flag_rem2 = 1;
        com5ON ();
        ui->pushButton_start_com5->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else{
        flag_rem2 = 0;
        com5OFF ();
        ui->pushButton_start_com5->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
}

void MainWindow::paintvalue ()
{

    double u1, u2;
    int er1, er2;
    u1 = readcom6U ();
    u2 = readcom5U ();
    er1 = readerr11I ();
    er2 = readerr4I ();
    if (er1 != 0) {
        flag_rem1 = 0;
        ui->pushButton_start_com6->setChecked (false);
        ui->pushButton_start_com6->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    if (er2 != 0) {
        flag_rem2 = 0;
        ui->pushButton_start_com5->setChecked (false);
        ui->pushButton_start_com5->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    error_mes_2="";
    if (er1 == 1) error_mes_2="БП БУН НА: Превышение выходного порога по напряжению!\n";
    if (er1 == 2) error_mes_2="БП БУН НА: Превышение выходного порога по току!\n";
    if (er1 == 4) error_mes_2="БП БУН НА: Превышение выходного порога по мощности!\n";
    if (er1 == 8) error_mes_2="БП БУН НА: Превышение рабочей температуры!\n";
    if (er2 == 1) error_mes_2="БП ПНА: Превышение выходного порога по напряжению!\n";
    if (er2 == 2) error_mes_2="БП ПНА: Превышение выходного порога по току!\n";
    if (er2 == 4) error_mes_2="БП ПНА: Превышение выходного порога по мощности!\n";
    if (er2 == 8) error_mes_2="БП ПНА: Превышение рабочей температуры!\n";
    write_message();
    ui->U1out->setText (QString::number (u1 / 100));
    ui->U2out->setText (QString::number (u2 / 100));

}
void MainWindow::paintvaluebp ()
{
    double i1, i2;
    i1 = readcom6I ();
    i2 = readcom5I ();
    if (k > 998)
        k = 0;
    dat [k] = i1 / 100;
    dat1 [k] = i2 / 100;
    k++;
    plot_point ();
}
void MainWindow::on_pushButton_bp_clear_clicked ()
{
    k = 0;
}

void MainWindow::statusRS (QString data)
{
    QString buf;
    QStringList list3 = data.split (" ");
    int s = list3.size ();
    for (int i = 0; i < s - 1; i++) {
        buf += "0x";
        char hex = list3 [i].toInt ();
        buf += QString::number (hex, 16);
        buf += "\n";
        ui->tech_buf->setText (buf);
    }
}
void MainWindow::statusCAN (QString data)
{
    QString buf;
    QStringList list3 = data.split (" ");
    int s = list3.size ();
    for (int i = 0; i < s - 1; i++) {
        buf += "0x";
        char hex = list3 [i].toInt ();
        buf += QString::number (hex, 16);
        buf += "\n";
        ui->tech_buf_2->setText (buf);
    }
}
void MainWindow::on_pushButton_22_clicked()
{
    connect (this, SIGNAL (Tech_read(int )), myTech, SLOT (tech_read ( int )));
    emit Tech_read(2);
    disconnect (this, SIGNAL (Tech_read(int)), myTech, SLOT (tech_read ( int )));
}

void MainWindow::on_pushButton_23_clicked()
{
    connect (this, SIGNAL (Tech_read(int )), myTech, SLOT (tech_read ( int )));
    emit Tech_read(1);
    disconnect (this, SIGNAL (Tech_read(int)), myTech, SLOT (tech_read ( int )));
}
void MainWindow::statusM ()
{
    QString res;

    res += req_stm ();
    //res += req_tech ();
    connect (this, SIGNAL (OTD_req ()), myOTD, SLOT (OTD_req ()));
    emit OTD_req ();
    disconnect (this, SIGNAL (OTD_req ()), myOTD, SLOT (OTD_req ()));
    //if (res!="") ui->error_mod->setText (res);
    error_mes_3=res;
    write_message();
}
void MainWindow::status_OTD (QString data)
{
    /*if (data != "") {
        ui->error_mod->setText (data);
    }*/
    error_mes_4=data;
    write_message();
}
void MainWindow::write_message ()
{
    QString data;
    ui->error_mod->setStyleSheet ("font: 25 12pt GOST type A;" "color: red;");
    data+=error_m;
    data+=TestOutStr;
    data+= TestOutStr1;
    data+=error_mes_1;
    data+=error_mes_2;
    data+=error_mes_3;
    data+=error_mes_4;
    data+=error_mes_5;
    data+=error_mes_6;
    data+=error_mes_7;
    data+=error_mes_8;
    data+=error_mes_9;
    ui->error_mod->setText (data);
}
void MainWindow::plot_point ()
{
    double b = k; //Конец интервала, где рисуем график по оси Ox
    int N = k + 1; //Вычисляем количество точек, которые будем отрисовывать

    QVector<double> x (N), y (N), z (N); //Массивы координат точек
    double tra, tra1;
    int i = 0;
    QString S5 = ui->lineEdit_period_bp->text ();
    double t = S5.toDouble ()/1000;
    for (double X = 0; X < b; X++) {//Пробегаем по всем точкам
        tra = dat [i];
        tra1 = dat1 [i];
        x [i] = X * t;
        y [i] = tra;
        z [i] = tra1;
        i++;
    }
    QApplication::processEvents ();
    ui->widget->clearGraphs ();//Если нужно, то очищаем все графики
    ui->widget->addGraph ();
    ui->widget->graph (0)->setData (x, y);
    ui->widget->graph (0)->setPen (QPen (Qt::blue));
    ui->widget->addGraph ();
    ui->widget->graph (1)->setPen (QPen (Qt::red));
    ui->widget->graph (1)->setData (x, z);
    ui->widget->graph (0)->setName ("I(БУН НА),A ");
    ui->widget->graph (1)->setName ("I(ПНА),A ");
    ui->widget->xAxis->setTickLabelFont (QFont (QFont ().family (), 10));
    ui->widget->yAxis->setTickLabelFont (QFont (QFont ().family (), 10));
    ui->widget->xAxis->setLabelFont (QFont (QFont ().family (), 10));
    ui->widget->yAxis->setLabelFont (QFont (QFont ().family (), 10));
    ui->widget->xAxis->setLabel ("Время, с");
    ui->widget->yAxis->setLabel (" I изм, А");
    QString S3 = ui->lineEdit_bp_l1->text ();
    double l1 = S3.toDouble ();
    QString S4 = ui->lineEdit_bp_l2->text ();
    double l2 = S4.toDouble ();
    ui->widget->xAxis->setRange (l2, l1);//Для оси Ox
    QString S1 = ui->lineEdit_bp_max->text ();
    double m1 = S1.toDouble ();
    QString S2 = ui->lineEdit_bp_min->text ();
    double m2 = S2.toDouble ();
    ui->widget->yAxis->setRange (m2, m1);//Для оси Oy
    ui->widget->legend->setVisible (true);
    ui->widget->replot ();
}

void MainWindow::on_pushButton_U1_clicked ()
{
    QString S1 = ui->setU1->text ();
    double u1 = S1.toDouble ();
    QString S3 = ui->setlimU1->text ();
    double ul1 = S3.toDouble ();
    QString S4 = ui->setlimI1->text ();
    double Il1 = S4.toDouble ();
    if (u1>36 || ul1>36){
        if(ul1>36)ul1=36;
        if(u1>36)u1=36;
        ui->setU1->setText (QString::number (u1));
        ui->setlimU1->setText (QString::number (ul1));
        QMessageBox msgBox1;
        msgBox1.setWindowTitle("Внимание");
        msgBox1.setIcon(QMessageBox::Information);
        msgBox1.setText("Введенное напряжение выходит за рабочий диапазон!\n Установлено максимальное напряжение!");
        msgBox1.exec();
    }
    if (u1 > ul1){
        QMessageBox msgBox2;
        msgBox2.setWindowTitle("Внимание");
        msgBox2.setIcon(QMessageBox::Information);
        msgBox2.setText("Введенное напряжение превышает значение ограничения по напряжению!\n Установлено значение ограничения по напряжению!");
        msgBox2.exec();
        u1 = ul1;
    }

    if((u1>30 || ul1>30)&& u1<36 && ul1<36){
        int n = QMessageBox::warning(0,
                                     "Внимание",
                                     "Установлен высокий уровень выходного напряжения! "
                                     "\n Применить текущие настройки?",
                                     "Да",
                                     "Нет",
                                     QString(),
                                     0,
                                     1);
        if(!n) {
            ui->setU1->setText (QString::number (u1));
            setoverUIcom6 (ul1, Il1);
            setUIcom6 (u1);
        }}

    else{
        ui->setU1->setText (QString::number (u1));
        setoverUIcom6 (ul1, Il1);
        setUIcom6 (u1);
    }
}

void MainWindow::on_pushButton_U2_clicked ()
{
    QString S1 = ui->setU2->text ();
    double u1 = S1.toDouble ();
    QString S3 = ui->setlimU2->text ();
    double ul1 = S3.toDouble ();
    QString S4 = ui->setlimI2->text ();
    double Il1 = S4.toDouble ();
    if (u1>36 || ul1>36){
        if(ul1>36)ul1=36;
        if(u1>36)u1=36;
        ui->setU2->setText (QString::number (u1));
        ui->setlimU2->setText (QString::number (ul1));
        QMessageBox msgBox1;
        msgBox1.setWindowTitle("Внимание");
        msgBox1.setIcon(QMessageBox::Information);
        msgBox1.setText("Введенное напряжение выходит за рабочий диапазон!\n Установлено максимальное напряжение!");
        msgBox1.exec();
    }
    if (u1 > ul1){
        QMessageBox msgBox2;
        msgBox2.setWindowTitle("Внимание");
        msgBox2.setIcon(QMessageBox::Information);
        msgBox2.setText("Введенное напряжение превышает значение ограничения по напряжению!\n Установлено значение ограничения по напряжению!");
        msgBox2.exec();
        u1 = ul1;
    }
    if((u1>30 || ul1>30)&& u1<36 && ul1<36){
        int n = QMessageBox::warning(0,
                                     "Внимание",
                                     "Установлен высокий уровень выходного напряжения! "
                                     "\n Применить текущие настройки?",
                                     "Да",
                                     "Нет",
                                     QString(),
                                     0,
                                     1);
        if(!n) {
            ui->setU2->setText (QString::number (u1));
            setoverUIcom6 (ul1, Il1);
            setUIcom6 (u1);
        }}

    else{
        ui->setU2->setText (QString::number (u1));
        setoverUIcom5 (ul1, Il1);
        setUIcom5 (u1);
    }
}

void MainWindow::on_pushButton_2_clicked ()
{
    Reset_error_com6 ();
    ui->error_mod->setText (" ");
}

void MainWindow::on_pushButton_3_clicked ()
{
    Reset_error_com5 ();
    ui->error_mod->setText (" ");
}

void MainWindow::on_pushButton_4_clicked ()
{
    if (stm_on_com6 (1, 1) == 1 && flag_con1 == 0) {
        flag_con1 = 1;
        ui->pushButton_4->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
        ui->pushButton_14->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (stm_on_com6 (1, 0) == 1 && flag_con1 == 1) {
        flag_con1 = 0;
        ui->pushButton_4->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_14->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
}
void MainWindow::on_pushButton_7_clicked ()
{
    if (stm_on_com6 (2, 1) == 1 && flag_con3 == 0) {
        flag_con3 = 1;
        ui->pushButton_7->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
        ui->pushButton_15->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (stm_on_com6 (2, 0) == 1 && flag_con3 == 1) {
        flag_con3 = 0;
        ui->pushButton_7->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_15->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
}
void MainWindow::on_pushButton_14_clicked()
{
    on_pushButton_4_clicked ();
}
void MainWindow::on_pushButton_15_clicked()
{
    on_pushButton_7_clicked ();
}
void MainWindow::on_pushButton_check_fuse_4_clicked()
{
    on_pushButton_check_fuse_1_clicked ();
}
void MainWindow::on_pushButton_check_fuse_3_clicked()
{
    on_pushButton_check_fuse_2_clicked ();
}
void MainWindow::on_pushButton_5_clicked ()
{
    if (stm_on_com5 (4, 1) == 1 && flag_con2 == 0) {
        flag_con2 = 1;
        ui->pushButton_5->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
        ui->pushButton_18->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
        if (flag_otd_auto == 0)
            on_OTD_avt_2_clicked ();
    }
    else if (stm_on_com5 (4, 0) == 1 && flag_con2 == 1) {
        flag_con2 = 0;
        ui->pushButton_5->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_18->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
}

void MainWindow::on_pushButton_8_clicked ()
{
    if (stm_on_com5 (5, 1) == 1 && flag_con4 == 0) {
        flag_con4 = 1;
        ui->pushButton_8->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
        ui->pushButton_16->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
        if (flag_otd_auto == 0)
            on_OTD_avt_2_clicked ();
    }
    else if (stm_on_com5 (5, 0) == 1 && flag_con4 == 1) {
        flag_con4 = 0;
        ui->pushButton_8->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_16->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
}

void MainWindow::on_pushButton_10_clicked ()
{
    if (stm_on_com5 (6, 1) == 1 && flag_con5 == 0) {
        flag_con5 = 1;
        ui->pushButton_10->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
        ui->pushButton_17->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (stm_on_com5 (6, 0) == 1 && flag_con5 == 1) {
        flag_con5 = 0;
        ui->pushButton_10->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_17->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
}
void MainWindow::on_pushButton_18_clicked()
{
    on_pushButton_5_clicked ();
}
void MainWindow::on_pushButton_16_clicked()
{
    on_pushButton_8_clicked ();
}
void MainWindow::on_pushButton_17_clicked()
{
    on_pushButton_10_clicked ();
}
void MainWindow::on_pushButton_ctm_ch_0_clicked ()
{
    double res = ctm_data_ch (0) / 10000;
    ui->lineEdit_ctm_ch_0->setText (QString::number (res));
    if (res >= 2) {
        ui->lineEdit_ctm_ch_0->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
    }
    else if (res >= 0 && res < 2 && req_stm () == "") {
        ui->lineEdit_ctm_ch_0->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 230, 0);"));
    }
}
void MainWindow::on_pushButton_ctm_ch_1_clicked ()
{
    double res = ctm_data_ch (1) / 10000;
    ui->lineEdit_ctm_ch_1->setText (QString::number (res));
    if (res >= 2) {
        ui->lineEdit_ctm_ch_1->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
    }
    else if (res >= 0 && res < 2 && req_stm () == "") {
        ui->lineEdit_ctm_ch_1->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 230, 0);"));
    }
}
void MainWindow::on_pushButton_ctm_ch_2_clicked ()
{
    double res = ctm_data_ch (2) / 10000;
    ui->lineEdit_ctm_ch_2->setText (QString::number (res));
    if (res >= 2) {
        ui->lineEdit_ctm_ch_2->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
    }
    else if (res >= 0 && res < 2 && req_stm () == "") {
        ui->lineEdit_ctm_ch_2->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 230, 0);"));
    }
}

void MainWindow::on_pushButton_ctm_ch_3_clicked ()
{
    double res = ctm_data_ch (3) / 10000;
    ui->lineEdit_ctm_ch_3->setText (QString::number (res));
    if (res >= 2) {
        ui->lineEdit_ctm_ch_3->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
    }
    else if (res >= 0 && res < 2 && req_stm () == "") {
        ui->lineEdit_ctm_ch_3->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 230, 0);"));
    }
}

void MainWindow::on_pushButton_ctm_ch_4_clicked ()
{
    on_pushButton_ctm_ch_0_clicked ();
    on_pushButton_ctm_ch_1_clicked ();
    on_pushButton_ctm_ch_2_clicked ();
    on_pushButton_ctm_ch_3_clicked ();
}
void MainWindow::on_pushButton_check_fuse_1_clicked ()
{
    int cf = ctm_check_fuse (1) + ctm_check_fuse (2);
    ui->pushButton_check_fuse_1->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
    ui->pushButton_check_fuse_4->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
    if (cf == 0)      {
        error_mes_7=("");
        ui->pushButton_check_fuse_1->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 230, 0);"));
        ui->pushButton_check_fuse_4->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 230, 0);"));
    }
    else if (cf == 1 || cf == 2) error_mes_7=("СТМ: Основной предохранитель неисправен!\n");
    else if (cf == 3 || cf == 4) error_mes_7=("СТМ: Ошибка при проверке основного предохранителя!\n");
    write_message();
}
void MainWindow::on_pushButton_check_fuse_2_clicked ()
{
    int cf = ctm_check_fuse (3) + ctm_check_fuse (4);
    ui->pushButton_check_fuse_2->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
    ui->pushButton_check_fuse_3->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
    if (cf == 0) {
        error_mes_8=("");
        ui->pushButton_check_fuse_2->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 230, 0);"));
        ui->pushButton_check_fuse_3->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 230, 0);"));
    }
    else if (cf == 1 || cf == 2) error_mes_8=("СТМ: Резервный предохранитель неисправен!\n");
    else if (cf == 3 || cf == 4) error_mes_8=("СТМ: Ошибка при проверке резервного предохранителя!\n");
    write_message();
}

void MainWindow::on_pushButton_tech_fd_clicked ()
{
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(36, 0, 0);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
}

void MainWindow::on_pushButton_tech_hd_clicked ()
{
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(36, 1, 0);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
}

void MainWindow::on_tech_set_speed_clicked ()
{
    int sp = ui->baudRateBox_tech->currentIndex ();
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(37, sp, 0);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
}

void MainWindow::on_pushButton_send_tech_clicked ()
{
    QString S1;
    S1 = ui->str_tech->text ();
    QStringList list1 = S1.split (" ");
    int s = list1.size ();
    int cou = 0;
    for (int i = 0; i < s; i++) {
        if (list1 [i] == "") {
            cou = cou + 1;
        }
    }
    list1.removeAll ("");
    s = s - cou;
    int x = (s >> 8) & 0xFF;
    int y = s & 0xFF;
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(22, x, y);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    bool ok;
    int hex;
    for (int i = 0; i < s; i++) {
        if (list1 [i] != "") {
            hex = list1 [i].toInt (&ok, 16);
            if (ok == 1 && hex < 256) {
                connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
                emit Tech_send(23, hex, 0);
                disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
            }
            else{
                i = s;
                Tech_error (630);
            }
        }
    }
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(24, 0, 0);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
}

void MainWindow::on_tech_clear_out_3_clicked ()
{
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(27, 1, 0);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
}

void MainWindow::on_tech_clear_in_3_clicked ()
{
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(27, 2, 0);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
}

void MainWindow::on_tech_clear_buf_3_clicked ()
{
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(27, 3, 0);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
}

void MainWindow::on_tech_clear_out_4_clicked ()
{
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(21, 1, 0);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
}

void MainWindow::on_tech_clear_in_4_clicked ()
{
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(21, 2, 0);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
}

void MainWindow::on_tech_clear_buf_4_clicked ()
{
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(21, 3, 0);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
}

void MainWindow::on_pushButton_send_tech_2_clicked ()
{
    QString S1;

    S1 = ui->str_tech_2->text ();
    QStringList list1 = S1.split (" ");
    int s = list1.size ();
    int cou = 0;
    for (int i = 0; i < s; i++) {
        if (list1 [i] == "") {
            cou = cou + 1;
        }
    }
    list1.removeAll ("");
    s = s - cou;
    int x = (s >> 8) & 0xFF;
    int y = s & 0xFF;
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(16, x, y);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    bool ok;
    int hex;
    for (int i = 0; i < s; i++) {
        if (list1 [i] != "") {
            hex = list1 [i].toInt (&ok, 16);
            if (ok == 1 && hex < 256) {
                connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
                emit Tech_send(17, hex, 0);
                disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
            }
            else{
                i = s;
                Tech_error (630);
            }
        }
    }
    connect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
    emit Tech_send(18, 0, 0);
    disconnect (this, SIGNAL (Tech_send(int , int , int)), myTech, SLOT (tech_send (int , int , int )));
}

void MainWindow::on_res_err_stm_clicked ()
{
    if (res_err_stm () != 1){
        error_mes_5= ("СТМ: Не удалось сбросить ошибку!\n");
        write_message();
    }
}
void MainWindow::on_pushButton_19_clicked()
{
    if( echo_com()==1){
        ui->pushButton_19->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else {
        ui->pushButton_19->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
        error_mes_5= ("CТМ: Ошибка при исполнении команды 'Эхо'!\n");
        write_message();
    }
}
void MainWindow::on_res_err_otd_clicked ()
{
    connect (this, SIGNAL (OTD_err ()), myOTD, SLOT (err_res_OTD ()));
    emit OTD_err ();
    disconnect (this, SIGNAL (OTD_res ()), myOTD, SLOT (err_res_OTD ()));
}
void MainWindow::OTD_err_res (int x)
{
    if (x != 1) {
        error_mes_6=("ОТД: Не удалось сбросить ошибку!\n");
        write_message();
    }
}

void MainWindow::on_res_err_tech_clicked ()
{
    //if (res_err_tech () != 1) ui->error_mod->setText ("ТЕХ: Не удалось сбросить ошибку!");
    connect (this, SIGNAL (Tech_res_err()), myTech, SLOT (res_err_tech ()));
    emit Tech_res_err();
    disconnect (this, SIGNAL (Tech_res_err()), myTech, SLOT (res_err_tech ()));
}

void MainWindow::on_pushButton_res_stm_clicked ()
{
    if (res_stm () != 1) {
        error_mes_5= ("СТМ: Не удалось провести перезагрузку СТМ!\n");
        write_message();
    }
}

void MainWindow::on_pushButton_res_tech_clicked ()
{
    connect (this, SIGNAL (Tech_res()), myTech, SLOT (res_tech ()));
    emit Tech_res();
    disconnect (this, SIGNAL (Tech_res()), myTech, SLOT (res_tech ()));
}
void MainWindow::on_pushButton_res_otd_clicked ()
{
    connect (this, SIGNAL (OTD_res ()), myOTD, SLOT (res_OTD ()));
    emit OTD_res ();
    disconnect (this, SIGNAL (OTD_res ()), myOTD, SLOT (res_OTD ()));
}
void MainWindow::on_pushButton_20_clicked()
{
    connect (this, SIGNAL (OTD_echo ()), myOTD, SLOT (echo_OTD ()));
    emit OTD_echo ();
    disconnect (this, SIGNAL (OTD_echo ()), myOTD, SLOT (echo_OTD ()));
}
void MainWindow::OTD_ind_echo(int x)
{
    if( x==1){
        ui->pushButton_20->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else {
        ui->pushButton_20->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
        error_mes_6= ("ОТД: Ошибка при исполнении команды 'Эхо'!\n");
        write_message();
    }
}
void MainWindow::OTD_res_st (int x)
{
    if (x != 1) {
        error_mes_6= ("ОТД: Не удалось провести перезагрузку!\n");
        write_message();
    }
}
void MainWindow::OTD_id ()
{
    error_mes_6= ("ОТД: Модуль установлен не в свой слот!\n");
    write_message();
}
void MainWindow::on_pushButton_6_clicked ()
{
    double v = fw_stm ();

    if (v == 2){
        error_mes_5= ("СТМ: Не удалось узнать версию прошивки!\n");
        write_message();
    }
    else ui->lineEdit->setText (QString::number (v / 10));
}

void MainWindow::on_pushButton_9_clicked ()
{
    connect (this, SIGNAL (Tech_fw ()), myTech, SLOT (fw_tech ()));
    emit Tech_fw ();
    disconnect (this, SIGNAL (Tech_fw ()), myTech, SLOT (fw_tech ()));
}
void MainWindow::on_pushButton_21_clicked()
{
    connect (this, SIGNAL (Tech_echo ()), myTech, SLOT (echo_tech ()));
    emit Tech_echo ();
    disconnect (this, SIGNAL (Tech_echo ()), myTech, SLOT (echo_tech ()));
}
void MainWindow::Tech_error (double x)
{
    if (x>100&& x!=333) error_mes_9 = "";
    if (x == 2) error_mes_9 = ("ТЕХ: Не удалось узнать версию прошивки!");
    if (x==100) error_mes_9 = ("ТЕХ: Не удалось сбросить ошибку!");
    if (x==200) error_mes_9 = ("ТЕХ: Не удалось провести перезагрузку!");
    if (x==400) error_mes_9 = ("ТЕХ: Oшибка при очистке буфера!");
    if (x==450) error_mes_9 = ("ТЕХ: Oшибка при очистке буферов!");
    if (x==500) error_mes_9 = ("ТЕХ: Oшибка при установке режима работы!");
    if (x==550) error_mes_9 = ("ТЕХ: Oшибка при установке скорости работы!");
    if (x==600) error_mes_9 = ("ТЕХ: Oшибка при формировании посылки данных!");
    if (x==630) error_mes_9 = ("ТЕХ: Oшибка во входящих данных!");
    if (x==650) error_mes_9 = ("ТЕХ: Oшибка при передаче посылки данных!");
    if (x==700) error_mes_9 = ("ТЕХ: Буфер пуст!");
    if (x==750) error_mes_9 = ("ТЕХ: Буфер переполнен!");
    if (x==800) error_mes_9 = ("ТЕХ: Oшибка амплитуды датчика RLM!");
    if (x==820) error_mes_9 = ("ТЕХ: Oшибка скорости датчика RLM!");
    if (x==840) error_mes_9 = ("ТЕХ: Oшибка конфигурации датчика RLM!");
    if (x==860) error_mes_9 = ("ТЕХ: Oшибка запроса датчика RLM!");
    if (x==0) error_mes_9 = ("");

    if (x==300) {
        error_mes_9 = ("ТЕХ: Ошибка при исполнении команды 'Эхо'!");
        ui->pushButton_20->setStyleSheet (QString::fromUtf8 ("background-color: rgb(250, 24, 0);"));
    }
    if (x==333) {
        ui->pushButton_21->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (x<100&&x!=0)ui->lineEdit_4->setText (QString::number (x / 10));
    write_message();
}
void MainWindow::on_pushButton_13_clicked ()
{
    connect (this, SIGNAL (OTD_sfw ()), myOTD, SLOT (OTD_fw ()));
    emit OTD_sfw ();
    disconnect (this, SIGNAL (OTD_sfw ()), myOTD, SLOT (OTD_fw ()));
}
void MainWindow::OTD_fw (double x)
{
    if (x == 2) {
        error_mes_6= ("ОТД: Не удалось узнать версию прошивки!\n");
        write_message();
    }
    else ui->lineEdit_5->setText (QString::number (x / 10));
}
void MainWindow::OTDPTdata (double x, double y)
{
    x = x / 100;
    y = y / 100;
    ui->error_mod->setStyleSheet ("font: 25 12pt GOST type A;" "color: red;");
    if (x == -256) {
        error_mes_6= ("ОТД: Ошибка измерения датчика ПТ-100!\n");
        write_message();
    }
    if (y == -256) {
        error_mes_6= ("ОТД: Ошибка измерения датчика ПТ-100!\n");
        write_message();
    }
    if (x > 80 || y > 80) {
        error_mes_6= ("ОТД: Превышение температуры! Отключение нагревателей!\n");
        if (stm_on_com5 (4, 0) == 1 && flag_con2 == 1) {
            flag_con2 = 0;
            ui->pushButton_5->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
            ui->pushButton_18->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        }
        if (stm_on_com5 (5, 0) == 1 && flag_con4 == 1) {
            flag_con4 = 0;
            ui->pushButton_8->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
            ui->pushButton_16->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        }
        write_message();
    }
    if (x > 1790) {
        error_mes_6= ("ОТД: Ошибка обращения к модулю датчика ПТ-100!\n");
        write_message();
    }
    if (y > 1790) {
        error_mes_6= ("ОТД: Ошибка обращения к модулю датчика ПТ-100!\n");
        write_message();
    }
    if(fabs(x)>0.0001||x==0)ui->OTDPT1->setText (QString::number (x));
    if(fabs(y)>0.0001||y==0)ui->OTDPT2->setText (QString::number (y));
}
void MainWindow::on_OTDPT_1_clicked()
{
    connect (this, SIGNAL (OTD1 (int)), myOTD, SLOT (OTDPT (int)));
    emit OTD1 (1);
    disconnect (this, SIGNAL (OTD1 (int)), myOTD, SLOT (OTDPT (int)));
}

void MainWindow::on_OTDPT_2_clicked()
{
    connect (this, SIGNAL (OTD1 (int)), myOTD, SLOT (OTDPT (int)));
    emit OTD1 (2);
    disconnect (this, SIGNAL (OTD1 (int)), myOTD, SLOT (OTDPT (int)));
}
void MainWindow::on_OTDPT_clicked ()
{
    connect (this, SIGNAL (OTD1 (int)), myOTD, SLOT (OTDPT (int)));
    emit OTD1 (3);
    disconnect (this, SIGNAL (OTD1 (int)), myOTD, SLOT (OTDPT (int)));
}

void MainWindow::OTDtemd (QString data)
{
    ui->OTDtd->setText (data);
}

void MainWindow::on_OTD_reset_1_clicked ()
{
    //myOTD->moveToThread(threadOTD);
    connect (this, SIGNAL (OTD_reset1 ()), myOTD, SLOT (OTDres1 ()));
    emit OTD_reset1 ();
    disconnect (this, SIGNAL (OTD_reset1 ()), myOTD, SLOT (OTDres1 ()));
}

void MainWindow::on_OTD_reset_2_clicked ()
{
    connect (this, SIGNAL (OTD_reset2 ()), myOTD, SLOT (OTDres2 ()));
    emit OTD_reset2 ();
    disconnect (this, SIGNAL (OTD_reset2 ()), myOTD, SLOT (OTDres2 ()));
}
void MainWindow::OTDerror (QString err)
{
    ui->error_mod->setStyleSheet ("font: 25 12pt GOST type A;" "color: red;");
    error_mes_6=err;
    write_message();
    //ui->error_mod->setText (err);
}

void MainWindow::on_OTD_meas_1_clicked ()
{
    connect (this, SIGNAL (OTD_meas1 ()), myOTD, SLOT (OTDmeas1 ()));
    emit OTD_meas1 ();
    disconnect (this, SIGNAL (OTD_meas1 ()), myOTD, SLOT (OTDmeas1 ()));
}

void MainWindow::on_OTD_meas_2_clicked ()
{
    connect (this, SIGNAL (OTD_meas2 ()), myOTD, SLOT (OTDmeas2 ()));
    emit OTD_meas2 ();
    disconnect (this, SIGNAL (OTD_meas2 ()), myOTD, SLOT (OTDmeas2 ()));
}

void MainWindow::OTDtm1 (QString temp)
{
    ui->OTDtm1->setText (temp);
}
void MainWindow::OTDtm2 (QString temp)
{
    ui->OTDtm2->setText (temp);
}
void MainWindow::OTDtm_err ()
{
    error_mes_6= ("ОТД: Превышение температуры! Отключение нагревателей!\n");
    if (stm_on_com5 (4, 0) == 1 && flag_con2 == 1) {
        flag_con2 = 0;
        ui->pushButton_5->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_18->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    if (stm_on_com5 (5, 0) == 1 && flag_con4 == 1) {
        flag_con4 = 0;
        ui->pushButton_8->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_16->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    write_message();
}
void MainWindow::on_OTD_nd_clicked ()
{
    //myOTD->moveToThread(threadOTD);
    connect (this, SIGNAL (OTD_nd ()), myOTD, SLOT (OTDtemper ()));
    emit OTD_nd ();
    disconnect (this, SIGNAL (OTD_nd ()), myOTD, SLOT (OTDtemper ()));
}

void MainWindow::on_pow_DY_osn_clicked ()
{
    QString S1 = ui->lineEdit_Addr_2->text ();
    int u1 = S1.toInt ();

    connect (this, SIGNAL (MKO_DY (int,int)), myMKO, SLOT (pow_DY (int,int)));
    emit MKO_DY (0, u1);
    disconnect (this, SIGNAL (MKO_DY (int,int)), myMKO, SLOT (pow_DY (int,int)));
}

void MainWindow::on_pow_DY_rez_clicked ()
{
    QString S2 = ui->lineEdit_Addr_3->text ();
    int u2 = S2.toInt ();

    connect (this, SIGNAL (MKO_DY (int,int)), myMKO, SLOT (pow_DY (int,int)));
    emit MKO_DY (1, u2);
    disconnect (this, SIGNAL (MKO_DY (int,int)), myMKO, SLOT (pow_DY (int,int)));
}

void MainWindow::on_MKO_osn_clicked ()
{
    if (flag_mko_osn == 0) {
        flag_mko_osn = 1;
        ui->MKO_osn->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (flag_mko_osn == 1) {
        flag_mko_osn = 0;
        ui->MKO_osn->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    connect (this, SIGNAL (MKO_ch (int)), myMKO, SLOT (MKO_chan (int)));
    emit MKO_ch (flag_mko_osn + flag_mko_rez);
    disconnect (this, SIGNAL (MKO_ch (int)), myMKO, SLOT (MKO_chan (int)));
}

void MainWindow::on_MKO_rez_clicked ()
{
    if (flag_mko_rez == 0) {
        flag_mko_rez = 2;
        ui->MKO_rez->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (flag_mko_rez == 2) {
        flag_mko_rez = 0;
        ui->MKO_rez->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    connect (this, SIGNAL (MKO_ch (int)), myMKO, SLOT (MKO_chan (int)));
    emit MKO_ch (flag_mko_osn + flag_mko_rez);
    disconnect (this, SIGNAL (MKO_ch (int)), myMKO, SLOT (MKO_chan (int)));
}

void MainWindow::on_MKO_test_clicked ()
{
    QString S1 = ui->lineEdit_Addr_2->text ();
    int u1 = S1.toInt ();
    QString S2 = ui->lineEdit_Addr_3->text ();
    int u2 = S2.toInt ();
    flag_mko_test=1;
    connect (this, SIGNAL (MKO_ts (int,int,int)), myMKO, SLOT (MKO_start_test (int,int,int)));
    emit MKO_ts (flag_mko_osn + flag_mko_rez, u1, u2);
    disconnect (this, SIGNAL (MKO_ts (int,int,int)), myMKO, SLOT (MKO_start_test (int,int,int)));
}

void MainWindow::on_pushButton_11_clicked ()
{
    QString S1;
    S1 += ui->MKO_cd_0->text () + " ";
    S1 += ui->MKO_cd_1->text () + " ";
    S1 += ui->MKO_cd_3->text () + " ";
    S1 += ui->MKO_cd_4->text () + " ";
    S1 += ui->MKO_cd_5->text () + " ";
    S1 += ui->MKO_cd_6->text () + " ";
    S1 += ui->MKO_cd_8->text () + " ";
    S1 += ui->MKO_cd_9->text ();
    if (ui->radioButton_2->isChecked()==1){
        flag_mko_osn=flag_mko_osn+10;
    }
    QString S3 = ui->lineEdit_Addr_2->text ();
    int u1 = S3.toInt ();
    QString S2 = ui->lineEdit_Addr_3->text ();
    int u2 = S2.toInt ();
    connect (this, SIGNAL (MKO_cm (int,QString,int,int)), myMKO, SLOT (MKO_tr_cm (int,QString,int,int)));
    emit MKO_cm (flag_mko_osn + flag_mko_rez, S1, u1, u2);
    disconnect (this, SIGNAL (MKO_cm (int,QString,int,int)), myMKO, SLOT (MKO_tr_cm (int,QString,int,int)));
    if (ui->radioButton_2->isChecked()==1){
        flag_mko_osn=flag_mko_osn-10;
    }
}

void MainWindow::on_pushButton_12_clicked ()
{
    QString S1 = ui->lineEdit_Addr_2->text ();
    int u1 = S1.toInt ();
    QString S2 = ui->lineEdit_Addr_3->text ();
    int u2 = S2.toInt ();

    connect (this, SIGNAL (MKO_cm_r (int,int,int)), myMKO, SLOT (MKO_rc_cm (int,int,int)));
    emit MKO_cm_r (flag_mko_osn + flag_mko_rez, u1, u2);
    disconnect (this, SIGNAL (MKO_cm_r (int,int,int)), myMKO, SLOT (MKO_rc_cm (int,int,int)));
}


void MainWindow::MKO_change_ch (int x, int y)
{
    stm_on_mko (x, y);
}

void MainWindow::on_MKO_avt_clicked ()
{
    QString S1 = ui->lineEdit_Addr_2->text ();
    int u1 = S1.toInt ();
    QString S2 = ui->lineEdit_Addr_3->text ();
    int u2 = S2.toInt ();
    QString S3 = ui->lineEdit_period->text ();
    int u3 = S3.toInt ();

    if (flag_mko_auto == 0) {
        flag_mko_auto = 1;
        ui->MKO_avt->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
        connect (this, SIGNAL (MKO_auto (int,int,int,int)), myMKO, SLOT (MKO_avt (int,int,int,int)));
        emit MKO_auto (flag_mko_auto, u3, u1, u2);
        disconnect (this, SIGNAL (MKO_auto (int,int,int,int)), myMKO, SLOT (MKO_avt (int,int,int,int)));
    }
    else if (flag_mko_auto == 1) {
        flag_mko_auto = 0;
        ui->MKO_avt->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        connect (this, SIGNAL (MKO_auto (int,int,int,int)), myMKO, SLOT (MKO_avt (int,int,int,int)));
        emit MKO_auto (flag_mko_auto, u3, u1, u2);
        disconnect (this, SIGNAL (MKO_auto (int,int,int,int)), myMKO, SLOT (MKO_avt (int,int,int,int)));
    }
}

void MainWindow::on_OTD_avt_2_clicked ()
{
    QString S3 = ui->lineEdit_period_OTD->text ();
    int u3 = S3.toInt ();

    if (flag_otd_auto == 0) {
        flag_otd_auto = 1;
        ui->OTD_avt_2->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
        connect (this, SIGNAL (OTD_auto (int,int)), myOTD, SLOT (OTD_avt (int,int)));
        emit OTD_auto (flag_otd_auto, u3 * 1000);
        disconnect (this, SIGNAL (OTD_auto (int,int)), myOTD, SLOT (OTD_avt (int,int)));
    }
    else if (flag_otd_auto == 1) {
        flag_otd_auto = 0;
        ui->OTD_avt_2->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        connect (this, SIGNAL (OTD_auto (int,int)), myOTD, SLOT (OTD_avt (int,int)));
        emit OTD_auto (flag_otd_auto, u3);
        disconnect (this, SIGNAL (OTD_auto (int,int)), myOTD, SLOT (OTD_avt (int,int)));
    }
}

void MainWindow::on_bp_cyc_clicked ()
{
    QString S3 = ui->lineEdit_period_bp->text ();
    double u3 = S3.toDouble ();

    if (flag_bp_auto == 0) {
        flag_bp_auto = 1;
        ui->bp_cyc->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
        connect (this, SIGNAL (bp_auto (int,int)), myBP, SLOT (bp_avt (int,int)));
        emit bp_auto (flag_bp_auto, u3);
        disconnect (this, SIGNAL (bp_auto (int,int)), myBP, SLOT (bp_avt (int,int)));
    }
    else if (flag_bp_auto == 1) {
        flag_bp_auto = 0;
        ui->bp_cyc->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        connect (this, SIGNAL (bp_auto (int,int)), myBP, SLOT (bp_avt (int,int)));
        emit bp_auto (flag_bp_auto, u3);
        disconnect (this, SIGNAL (bp_auto (int,int)), myBP, SLOT (bp_avt (int,int)));
    }
}

void MainWindow::on_tech_avto_clicked()
{
    QString S3 = ui->tech_period->text ();
    int u3 = S3.toInt ();
    if (flag_tech_auto == 0) {
        buf_SSI[0]="";
        flag_tech_auto = 1;
        ui->tech_avto->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
        connect (this, SIGNAL (Tech_auto (int,int)), myTech, SLOT (Tech_avt (int,int)));
        emit Tech_auto (flag_tech_auto, u3);
        disconnect (this, SIGNAL (Tech_auto (int,int)), myTech, SLOT (Tech_avt (int,int)));
    }
    else if (flag_tech_auto == 1) {
        flag_tech_auto = 0;
        ui->tech_avto->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        connect (this, SIGNAL (Tech_auto (int,int)), myTech, SLOT (Tech_avt (int,int)));
        emit Tech_auto (flag_tech_auto, u3);
        disconnect (this, SIGNAL (Tech_auto (int,int)), myTech, SLOT (Tech_avt (int,int)));
    }
}

void MainWindow::on_tech_single_shot_clicked()
{
    connect (this, SIGNAL (Tech_auto (int,int)), myTech, SLOT (Tech_avt (int,int)));
    emit Tech_auto (2, 1000);
    disconnect (this, SIGNAL (Tech_auto (int,int)), myTech, SLOT (Tech_avt (int,int)));
}


void MainWindow::Tech_SSI_value (double x, double y, double skvt_V, double skvt_H)
{
    ui->tech_SSI_dec->setText (QString::number (x));
    ui->tech_dec_v->setText (QString::number (skvt_V));
    ui->tech_dec_h->setText (QString::number (skvt_H));
    int a1=1,a2=1;
    if(flag_tech_ssi==1||flag_rm==1){
        a2=1;
        a1=1;
    }
    if(flag_rm_4==1||flag_rlm2==1){
        a2=192;
        a1=192;
    }
    if(flag_rlm==4){
        a2=192;
        a1=1;
    }
    double degr = 180*x/8192*a1;
    int degree = degr;
    double z1=(180*x/8192.0*a1-degree)*60.0;
    int minute =z1;
    int sec = (z1-minute)*60.0;
    double degr_RM = 180*y/8192*a2;
    int degree_RM = degr_RM;
    double z2=(180*y/8192.0*a2-degree_RM)*60.0;
    int minute_RM =z2;
    int sec_RM = (z2-minute_RM)*60.0;
    double degrv = 180*skvt_V/65536;
    int degreev = degrv;
    double zv=(180*skvt_V/65536.0-degreev)*60.0;
    int minutev =zv;
    int secv = (zv-minutev)*60.0;
    double degrh = 180*skvt_H/65536;
    int degreeh = degrh;
    double zh=(180*skvt_H/65536.0-degreeh)*60.0;
    int minuteh =zh;
    int sech = (zh-minuteh)*60.0;
    ui->tech_SSI_degree->setText (QString::number (degree));
    ui->tech_SSI_minute->setText (QString::number (minute));
    ui->tech_SSI_sec->setText (QString::number (sec));
    ui->tech_degree_v->setText (QString::number (degreev));
    ui->tech_minute_v->setText (QString::number (minutev));
    ui->tech_sec_v->setText (QString::number (secv));
    ui->tech_degree_h->setText (QString::number (degreeh));
    ui->tech_minute_h->setText (QString::number (minuteh));
    ui->tech_sec_h->setText (QString::number (sech));
    for(int k=19;k>0;k--)
    {
        buf_SSI[k] = buf_SSI[k-1];
        buf_RM_2[k] = buf_RM_2[k-1];
        buf_V[k] = buf_V[k-1];
        buf_H[k] = buf_H[k-1];
        delta_V[k] = delta_V[k-1];
        delta_H[k] = delta_H[k-1];
        delta_RM_2[k] = delta_RM_2[k-1];
    }
    if(buf_SSI[0]==""){
        d_v[0]=(degr-zv);
        d_h[0]=(degr-zh);
        d_h[1]= degr - degr_RM;
    }
    buf_SSI[0]="";
    buf_RM_2[0]="";
    buf_V[0]="";
    buf_H[0]="";
    delta_V[0]="";
    delta_H[0]="";
    delta_RM_2[0]="";
    buf_SSI[0] += QString::number (degree);
    buf_SSI[0] += "° ";
    buf_SSI[0] += QString::number (minute);
    buf_SSI[0] += "' ";
    buf_SSI[0] += QString::number (sec);
    buf_SSI[0] += "''";
    buf_RM_2[0] += QString::number (degree_RM);
    buf_RM_2[0] += "° ";
    buf_RM_2[0] += QString::number (minute_RM);
    buf_RM_2[0] += "' ";
    buf_RM_2[0] += QString::number (sec_RM);
    buf_RM_2[0] += "''";
    buf_V[0] += QString::number (degreev);
    buf_V[0] += "° ";
    buf_V[0] += QString::number (minutev);
    buf_V[0] += "' ";
    buf_V[0] += QString::number (secv);
    buf_V[0] += "''";
    buf_H[0] += QString::number (degreeh);
    buf_H[0] += "° ";
    buf_H[0] += QString::number (minuteh);
    buf_H[0] += "' ";
    buf_H[0] += QString::number (sech);
    buf_H[0] += "''";
    double check1 = degr-zv-d_v[0];
    check1=-180.99;
    if(check1 >100){
        check1=(check1-180);
    }
    if(check1 <-100){
        check1=(check1+180);
    }
    int d1 = check1;
    double z4=(check1-d1)*60.0;
    int m1 =z4;
    int s1 = (z4-m1)*60.0;
    double check2 = degr-zh-d_h[0];
    if(check2 >100){
        check2=(check2-180);
    }
    if(check2 <-100){
        check2=(check2+180);
    }
    int d2 = check2;
    double z5=(check2-d2)*60.0;
    int m2 =z5;
    int s2 = (z5-m2)*60.0;
    double check3 = degr - degr_RM-d_h[1];
    if(check3 >100){
        check3=(check3-180);
    }
    if(check3 <-100){
        check3=(check3+180);
    }
    int d3 = check3;
    double z6=(check3-d3)*60.0;
    int m3 =z6;
    int s3 = (z6-m3)*60.0;
    delta_V[0] += QString::number (d1);
    delta_V[0] += "° ";
    delta_V[0] += QString::number (m1);
    delta_V[0] += "' ";
    delta_V[0] += QString::number (s1);
    delta_V[0] += "''";
    delta_H[0] += QString::number (d2);
    delta_H[0] += "° ";
    delta_H[0] += QString::number (m2);
    delta_H[0] += "' ";
    delta_H[0] += QString::number (s2);
    delta_H[0] += "''";
    delta_RM_2[0] += QString::number (d3);
    delta_RM_2[0] += "° ";
    delta_RM_2[0] += QString::number (m3);
    delta_RM_2[0] += "' ";
    delta_RM_2[0] += QString::number (s3);
    delta_RM_2[0] += "''";

    delete_table_tech();
    for(int c=0;c<20;c++)
    {
        add_string_table_tech (c, buf_SSI[c], buf_RM_2[c],delta_RM_2[c],buf_V[c],buf_H[c],delta_V[c],delta_H[c]);
    }
}

void MainWindow::on_pushButton_clicked()
{
    if (flag_tech_ssi == 0) {
        flag_tech_ssi = 1;
        connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        emit Tech_ssi (flag_tech_ssi);
        disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        ui->pushButton->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (flag_tech_ssi == 1) {
        flag_tech_ssi = 0;
        ui->pushButton->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        emit Tech_ssi (flag_tech_ssi);
        disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
    }
}

void MainWindow::on_pushButton_log_clicked()
{
    QString S3 = ui->tech_log->text ();
    if (flag_tech_log == 0) {
        flag_tech_log = 1;
        connect (this, SIGNAL (Tech_log (int, QString)), myTech, SLOT (Tech_log (int, QString)));
        emit Tech_log (flag_tech_log+flag_rm+flag_rm_4*2,S3);
        disconnect (this, SIGNAL (Tech_log (int, QString)), myTech, SLOT (Tech_log (int, QString)));
        ui->pushButton_log->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (flag_tech_log == 1) {
        flag_tech_log = 0;
        ui->pushButton_log->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        connect (this, SIGNAL (Tech_log (int, QString)), myTech, SLOT (Tech_log (int, QString)));
        emit Tech_log (flag_tech_log,S3);
        disconnect (this, SIGNAL (Tech_log (int, QString)), myTech, SLOT (Tech_log (int, QString)));
    }
}
void MainWindow::on_pushButton_RM_2_clicked()
{
    if (flag_rm == 0) {
        flag_rm = 1;
        connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        emit Tech_ssi (2);
        disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        ui->pushButton_RM_2->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (flag_rm == 1) {
        flag_rm = 0;
        ui->pushButton_RM_2->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        emit Tech_ssi (0);
        disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
    }
}

void MainWindow::on_pushButton_rm4_1_clicked()
{
    if (flag_rm_4 == 0) {
        flag_rm_4 = 1;
        connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        emit Tech_ssi (3);
        disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        ui->pushButton_rm4_1->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (flag_rm_4 == 1) {
        flag_rm_4 = 0;
        ui->pushButton_rm4_1->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        emit Tech_ssi (0);
        disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
    }
}


void MainWindow::on_radioButton_3_clicked()
{
    connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
    emit Tech_ssi (5);
    disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
}

void MainWindow::on_radioButton_4_clicked()
{
    connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
    emit Tech_ssi (4);
    disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
}

void MainWindow::on_pushButton_25_clicked()
{
    if (flag_rlm2 == 0) {
        flag_rlm2 = 1;
        connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        emit Tech_ssi (5);
        disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        ui->pushButton_25->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (flag_rlm2 == 1) {
        flag_rlm2 = 0;
        ui->pushButton_25->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        emit Tech_ssi (0);
        disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
    }
}

void MainWindow::on_pushButton_24_clicked()
{
    if (flag_rlm == 0) {
        flag_rlm = 1;
        connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        emit Tech_ssi (4);
        disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        ui->pushButton_24->setStyleSheet (QString::fromUtf8 ("background-color: rgb(0, 255, 0);"));
    }
    else if (flag_rlm == 1) {
        flag_rlm = 0;
        ui->pushButton_24->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        connect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
        emit Tech_ssi (0);
        disconnect (this, SIGNAL (Tech_ssi (int)), myTech, SLOT (Tech_ssi (int)));
    }
}

void MainWindow::on_reset_clicked()
{
    connect (this, SIGNAL (MKO_ch (int)), myMKO, SLOT (MKO_chan (int)));
    emit MKO_ch (flag_mko_osn*10 + flag_mko_rez*10);
    disconnect (this, SIGNAL (MKO_ch (int)), myMKO, SLOT (MKO_chan (int)));
    if (flag_mko_osn == 1) {
        flag_mko_osn = 0;
        ui->MKO_osn->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    if (flag_mko_rez == 2) {
        flag_mko_rez = 0;
        ui->MKO_rez->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    stm_on_mko (1, 0);
    stm_on_mko (2, 0);
    startpower ();
    flag_rem1 = 0;
    ui->pushButton_start_com6->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    flag_rem2 = 0;
    ui->pushButton_start_com5->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    if (stm_on_com6 (1, 0) == 1 && flag_con1 == 1) {
        flag_con1 = 0;
        ui->pushButton_4->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_14->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    if (stm_on_com6 (2, 0) == 1 && flag_con3 == 1) {
        flag_con3 = 0;
        ui->pushButton_7->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_15->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    if (stm_on_com5 (4, 0) == 1 && flag_con2 == 1) {
        flag_con2 = 0;
        ui->pushButton_5->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_18->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    if (stm_on_com5 (5, 0) == 1 && flag_con4 == 1) {
        flag_con4 = 0;
        ui->pushButton_8->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_16->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    if (stm_on_com5 (6, 0) == 1 && flag_con5 == 1) {
        flag_con5 = 0;
        ui->pushButton_10->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
        ui->pushButton_17->setStyleSheet (QString::fromUtf8 ("background-color: rgb(230, 230, 230);"));
    }
    on_pushButton_19_clicked();
    on_pushButton_20_clicked();
    on_pushButton_21_clicked();
}
