#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include <QtSerialPort/QtSerialPort>
#include "myclass.h"
#include "OTD.h"
#include "MKO.h"
#include "comport.h"

int cErrors, fEventResult,strt;
int flag_rem1=0,flag_rem2=0,flag_con1=0, flag_con2=0,k,flag_con3=0,flag_con4=0,flag_con5=0;
int flag_mko_auto=0, flag_mko_osn=0,flag_mko_rez=0,flag_otd_auto=0;
int dat[1000]={0},dat1[1000]={0};
unsigned dwMRT;
int er;
uint16_t sum;
WORD tx_data[4];
QString TestOutStr,TestOutStr1,result_tech,error_m;
QString ncd, zcd, cdh, cdd;

QThread *threadOTD= new QThread;
OTD *myOTD = new OTD("B");

QThread *threadMKO= new QThread;
MKO *myMKO = new MKO("B");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_COMPortSender = new COMPortSender(this);

    startCondition(); // Ivan Semenchuk: uncomment to work with real device
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::startCondition()
{
    m_COMPortSender->createPorts();
    m_COMPortSender->startPower();

    ui->MKO_avt->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    ui->OTD_avt_2->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    ui->pushButton_start_com5->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    ui->pushButton_start_com6->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    ui->pushButton_4->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    ui->pushButton_5->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    ui->pushButton_7->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    ui->pushButton_8->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    ui->pushButton_10->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    ui->MKO_osn->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    ui->MKO_rez->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    ui->baudRateBox_tech->addItem(QStringLiteral("9600"));
    ui->baudRateBox_tech->addItem(QStringLiteral("19200"));
    ui->baudRateBox_tech->addItem(QStringLiteral("38400"));
    ui->baudRateBox_tech->addItem(QStringLiteral("57600"));
    ui->baudRateBox_tech->addItem(QStringLiteral("78125"));
    ui->baudRateBox_tech->addItem(QStringLiteral("115200"));
    ui->baudRateBox_tech->addItem(QStringLiteral("230400"));
    ui->baudRateBox_tech->addItem(QStringLiteral("460800"));
    ui->baudRateBox_tech->addItem(QStringLiteral("468750"));
    ui->baudRateBox_tech->addItem(QStringLiteral("625000"));
    ui->baudRateBox_tech->addItem(QStringLiteral("921600"));
    ui->baudRateBox_tech->addItem(QStringLiteral("1250000"));
    ui->baudRateBox_tech->addItem(QStringLiteral("3000000"));
    ui->lineEdit_Addr_2->setText(QString::number(0));
    ui->lineEdit_Addr_3->setText(QString::number(0));
    ui->lineEdit_period->setText(QString::number(1000));
    ui->lineEdit_period_OTD->setText(QString::number(1));
    ui->MKO_cd_0->setText(QString::number(0));
    ui->MKO_cd_1->setText(QString::number(0));
    ui->MKO_cd_3->setText(QString::number(0));
    ui->MKO_cd_4->setText(QString::number(0));
    ui->MKO_cd_5->setText(QString::number(0));
    ui->MKO_cd_6->setText(QString::number(0));
    ui->MKO_cd_8->setText(QString::number(0));
    ui->MKO_cd_9->setText(QString::number(0));
    ui->tableWidget_2->setColumnCount(4);
    ui->tableWidget_2->setColumnWidth(0,90);
    ui->tableWidget_2->setColumnWidth(1,200);
    ui->tableWidget_2->setColumnWidth(2,90);
    ui->tableWidget_2->setColumnWidth(3,90);
    ui->tableWidget_2->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Номер СД")));
    ui->tableWidget_2->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Значение разрядов СД")));
    ui->tableWidget_2->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Значение hex")));
    ui->tableWidget_2->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Значение dec")));
    ui->tableWidget_2->setShowGrid(true);
    ui->tableWidget_2->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_COMPortSender->stm_on_mko(1,0);
    m_COMPortSender->stm_on_mko(2,0);

    myMKO->moveToThread(threadMKO);
    connect(myMKO, SIGNAL(test_MKO(int)), this, SLOT(simpltst1(int)));
    connect(myMKO, SIGNAL(MKO_CTM(int, int)), this, SLOT(MKO_change_ch(int, int)));
    connect(myMKO, SIGNAL(start_MKO(QString)), this, SLOT(MKO_data(QString)));
    connect(myMKO, SIGNAL(data_MKO(QString)), this, SLOT(MKO_cm_data(QString)));
    threadMKO->start();

    myOTD->moveToThread(threadOTD);
    connect(myOTD, SIGNAL(start_OTDPT(double,double)), this, SLOT(OTDPTdata(double,double)));
    connect(myOTD, SIGNAL(temp_OTD(QString)), this, SLOT(OTDtemd(QString)));
    connect(myOTD, SIGNAL(OTD_res(int)), this, SLOT(OTD_res_st(int)));
    connect(myOTD, SIGNAL(OTD_reqr(QString)), this, SLOT(status_OTD(QString)));
    connect(myOTD, SIGNAL(OTD_err_res(int)), this, SLOT(OTD_err_res(int)));
    connect(myOTD, SIGNAL(OTD_id1()), this, SLOT(OTD_id()));
    connect(myOTD, SIGNAL(OTD_vfw(double)), this, SLOT(OTD_fw(double)));
    connect(myOTD, SIGNAL(err_OTD(QString)), this, SLOT(OTDerror(QString)));
    connect(myOTD, SIGNAL(tm_OTD1(QString)), this, SLOT(OTDtm1(QString)));
    connect(myOTD, SIGNAL(tm_OTD2(QString)), this, SLOT(OTDtm2(QString)));
    connect(threadOTD, SIGNAL(started()), myOTD, SLOT(COMConnectorOTD()));
    threadOTD->start();

    QThread *thread= new QThread;
    MyClass *my = new MyClass("B");
    my->moveToThread(thread);
    connect(my, SIGNAL(send()), this, SLOT(paintvalue()));
    connect(my, SIGNAL(send3()), this, SLOT(statusRS()));
    connect(my, SIGNAL(send4()), this, SLOT(statusCAN()));
    connect(my, SIGNAL(send5()), this, SLOT(statusM()));
    connect(thread, SIGNAL(started()), my, SLOT(doWork()));
    thread->start();
    //startpower(); // Ivan Semenchyk: what for to do it twice?

    ui->setU1->setText(QString::number(0.5));
    ui->setU2->setText(QString::number(0.5));
    ui->setlimU1->setText(QString::number(27));
    ui->setlimU2->setText(QString::number(27));
    ui->setlimI1->setText(QString::number(0.5));
    ui->setlimI2->setText(QString::number(0.5));
    ui->error_mod->setStyleSheet("font: 25 16pt GOST type A;" "color: red;");
    if(m_COMPortSender->id_stm() != 1)
    {
        error_m += " Модуль СТМ установлен не в свой слот!";
    }

    if(m_COMPortSender->id_tech() != 1)
    {
        error_m += " Модуль ТЕХНОЛОГИЧЕСКИЙ установлен не в свой слот!";
    }

    ui->error_mod->setText (error_m);
    error_m="";
}

int MainWindow::simpltst1(int z)
{
    if(z == 0)
    {
        ui->label_MKO->setStyleSheet("font: 25 10pt GOST type A;" "color: black;");
        TestOutStr += "Тест пройден успешно!\n";
        ui->label_MKO->setText(TestOutStr);
        TestOutStr = "";
    }
    else
    {
        ui->label_MKO->setStyleSheet("font: 25 10pt GOST type A;" "color: red;");
        TestOutStr1 += "Ошибка! Тест провален с ";
        TestOutStr1 += QString::number(z);
        TestOutStr1 += " ошибками. Перезагрузите программу!\n";
        ui->label_MKO->setText(TestOutStr1);
        TestOutStr1 = "";
    }

    return z ;
}

void MainWindow::delete_table_MKO()
{
    int n = ui->tableWidget_2->rowCount();
    for( int i = 0; i < n; i++ )
    {
        ui->tableWidget_2->removeRow( 0 );
    }
}

int MainWindow::add_string_table_MKO(int n, QString text_data, QString comm_data, QString data_hex, QString data_dec)
{
    ui->tableWidget_2->insertRow(n);
    ui->tableWidget_2->setItem(n, 0, new QTableWidgetItem(text_data));
    ui->tableWidget_2->setItem(n, 1, new QTableWidgetItem(comm_data));
    ui->tableWidget_2->setItem(n, 2, new QTableWidgetItem(data_hex));
    ui->tableWidget_2->setItem(n, 3, new QTableWidgetItem(data_dec));
    ui->tableWidget_2->setRowHeight(n, 35);
    return 1;
}

void MainWindow::MKO_data(QString data)
{
    ui->label_MKO->setStyleSheet("font: 25 10pt GOST type A;" "color: black;");
    ui->label_MKO->setText(data);
}

void MainWindow::MKO_cm_data(QString data)
{
    QStringList list1 = data.split(" ");
    QString list2[42];
    list2[0]+="Текущий режим работы ось ψ\n(от -32767 до +32767)";
    list2[1]+="Заданное положение в шагах ось ψ\n( от -214748648 до +214748648)";
    list2[2]+="Заданное положение в шагах ось ψ\n( от -214748648 до +214748648)";
    list2[3]+="Текущая скорость ось ψ\n ( от -32767 до +32767)";
    list2[4]+="Текущий ток ось ψ\n ( от -511 до +511)";
    list2[5]+="Текущий режим работы ось υ\n(от -32767 до +32767)";
    list2[6]+="Заданное положение в шагах ось υ\n( от -214748648 до +214748648) ";
    list2[7]+="Заданное положение в шагах ось υ\n( от -214748648 до +214748648)";
    list2[8]+="Текущая скорость ось υ\n ( от -32767 до +32767)";
    list2[9]+="Текущий ток ось υ\n ( от -511 до +511)";
    list2[10]+="Информация датчика угла по ψ\n (0…180град)";
    list2[11]+="Информация датчика угла по υ\n (0…180град) ";
    list2[12]+="Не используется";
    list2[13]+="Не используется";
    list2[14]+="Не используется";
    list2[15]+="Не используется";
    list2[16]+="Не используется";
    list2[17]+="Не используется";
    list2[18]+="Признак ошибки контрольной\n суммы принятого массива";
    list2[19]+="Информация термодатчика в град.\n (0xFFFF-датчика нет)";
    list2[20]+="Контрольная сумма";
    delete_table_MKO();
    ncd = "";
    zcd = "";
    cdh = "";
    cdd = "";
    int ct = 0;
    short a;
    if (list1[0] != "")
    {
        for(int i = 0; i < 21; i++)
        {
            int z=list1[i].toInt();
            ncd += "CД ";
            ncd += QString::number(i);
            zcd = list2[i];
            cdh += "0x";
            if (i < 10 && i != 1 && i != 6)
            {
                a = z;
                cdh += QString::number(z, 16);
                cdd += QString::number(a);
            }

            if (i == 10 || i == 11)
            {
                z = z * 180 / 65536;
                cdh += QString::number(z,16);
                cdd += QString::number(z);
            }

            if (i == 12)
            {
                i = i + 5;
                ncd +=" - СД " + QString::number(i);
                cdh += QString::number(z, 16);
                cdd += QString::number(z);
            }

            if (i == 18 || i == 20)
            {
                cdh += QString::number(z, 16);
                cdd += QString::number(z);
            }

            if (i == 19)
            {
                WORD y = z;
                y = y << 3;
                y = y >> 15;
                if (y == 1)
                {
                    y = z;
                    y = y << 4;
                    y = y >> 4;
                    z = y;
                }
                else
                {
                    z = 0xFFFF;
                }

                cdh += QString::number(z, 16);
                cdd += QString::number(z);
            }

            if (i ==1||i==6)
            {

                ncd += " - СД " + QString::number(i + 1);
                int f = list1[i].toInt() << 16;
                int k = f + list1[i + 1].toInt();
                z = f + list1[i + 1].toInt();
                i++;
                cdh += QString::number(z, 16);
                cdd += QString::number(k);
            }

            add_string_table_MKO(ct, ncd, zcd, cdh, cdd);
            ct++;
            ncd = "";
            zcd = "";
            cdh = "";
            cdd = "";
        }
    }

    if(flag_mko_osn + flag_mko_rez == 3)
    {
        if (list1[0] != "")
        {
            add_string_table_MKO(ct, "----------- ", "'РЕЗЕРВНЫЙ ПОЛУКОМПЛЕКТ'", "---------- ", "----------- ");
            ct++;

            for(int i = 0; i < 21; i++)
            {
                int z = list1[i + 21].toInt();
                ncd += "CД ";
                ncd += QString::number(i);
                zcd = list2[i];
                cdh += "0x";
                if (i < 10 && i != 1 && i != 6)
                {
                    a = z;
                    cdh += QString::number(z, 16);
                    cdd += QString::number(a);
                }

                if(i == 10 || i == 11)
                {
                    z = z * 180 / 65536;
                    cdh += QString::number(z, 16);
                    cdd += QString::number(z);
                }

                if(i == 12)
                {
                    i = i + 5;
                    ncd += " - СД " + QString::number(i);
                    cdh += QString::number(z, 16);
                    cdd += QString::number(z);
                }

                if(i == 18 || i == 20)
                {
                    cdh += QString::number(z, 16);
                    cdd += QString::number(z);
                }

                if(i == 19)
                {
                    WORD y = z;
                    y = y << 3;
                    y = y >> 15;
                    if(y == 1)
                    {
                        y = z;
                        y = y << 4;
                        y = y >> 4;
                        z = y;
                    }
                    else
                    {
                        z = 0xFFFF;
                    }

                    cdh += QString::number(z, 16);
                    cdd += QString::number(z);
                }

                if (i == 1 || i == 6)
                {
                    ncd += " - СД " + QString::number(i + 1);
                    int f = list1[i + 21].toInt() << 16;
                    int k = f + list1[i + 22].toInt();
                    z = f + list1[i + 22].toInt();
                    i++;
                    cdh += QString::number(z, 16);
                    cdd += QString::number(k);
                }

                add_string_table_MKO(ct, ncd, zcd, cdh, cdd);
                ct++;
                ncd = "";
                zcd = "";
                cdh = "";
                cdd = "";
            }
        }
    }
}

void MainWindow::on_pushButton_start_com6_clicked()
{
    if(flag_rem1 == 0)
    {
        flag_rem1 = 1;
        m_COMPortSender->setPowerState(COMPortSender::POW_ANT_DRV_CTRL, COMPortSender::POWER_ON);
        ui->pushButton_start_com6->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else
    {
        flag_rem1 = 0;
        m_COMPortSender->setPowerState(COMPortSender::POW_ANT_DRV_CTRL, COMPortSender::POWER_OFF);
        ui->pushButton_start_com6->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }
}

void MainWindow::on_pushButton_start_com5_clicked()
{
    if(flag_rem2 == 0)
    {
        flag_rem2 = 1;
        m_COMPortSender->setPowerState(COMPortSender::POW_ANT_DRV, COMPortSender::POWER_ON);
        ui->pushButton_start_com5->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else
    {
        flag_rem2 = 0;
        m_COMPortSender->setPowerState(COMPortSender::POW_ANT_DRV, COMPortSender::POWER_OFF);
        ui->pushButton_start_com5->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }
}

void MainWindow::paintvalue()
{
    double u1, u2, i1, i2;
    uint8_t er1, er2;

    ui->tech_error->setText("");
    m_COMPortSender->getCurVoltageAndCurrent(COMPortSender::POW_ANT_DRV, u1, i1, er1);
    m_COMPortSender->getCurVoltageAndCurrent(COMPortSender::POW_ANT_DRV_CTRL, u2, i2, er2);

    ui->err1->setStyleSheet("font: 25 9pt GOST type A;" "color: black;");
    ui->err2->setStyleSheet("font: 25 9pt GOST type A;" "color: black;");
    if(er1 != 0)
    {
        ui->err1->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        flag_rem1 = 0;
        ui->pushButton_start_com6->setChecked(false);
        ui->pushButton_start_com6->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }

    if(er2 != 0)
    {
        ui->err2->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        flag_rem2 = 0;
        ui->pushButton_start_com5->setChecked(false);
        ui->pushButton_start_com5->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }

    if(er1==1) ui->err1->setText("Overvoltage protection!");
    if(er1==2) ui->err1->setText("Overcurrent protection!");
    if(er1==4) ui->err1->setText("Overpower protection!");
    if(er1==8) ui->err1->setText("Overtemperature protection!");

    if(er2==1) ui->err2->setText("Overvoltage protection!");
    if(er2==2) ui->err2->setText("Overcurrent protection!");
    if(er2==4) ui->err2->setText("Overpower protection!");
    if(er2==8) ui->err2->setText("Overtemperature protection!");

    ui->U1out->setText(QString::number(u1 / 100));
    ui->U2out->setText(QString::number(u2 / 100));

    if(k > 500) // wtf?
    {
        k = 0;
    }

    dat[k] = i1 / 100;
    dat1[k] = i2 / 100;
    k++;
    plot_point();
}

void MainWindow::statusRS()
{
    int len1 = m_COMPortSender->tech_read(1);
    if(len1 != 0)
    {
        ui->tech_error->setText(" ");
        result_tech= m_COMPortSender->tech_read_buf(1, len1);
        QString buf;
        QStringList list2 = result_tech.split(" ");
        int s=list2.size ();
        for(int i = 0; i < s - 1; i++)
        {
            if(list2[i]=="em")
            {
                ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                ui->tech_error->setText(" Буфер пуст");
            }

            if(list2[i]=="uu")
            {
                ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                ui->tech_error->setText(" Буфер переполнен");
            }
            else
            {
                buf+="0x";
                char hex = list2[i].toInt();
                buf+=QString::number(hex,16);
                buf+="\n";
                ui->tech_buf->setText(buf);
            }
        }
    }
}
void MainWindow::statusCAN ()
{
    int len2 = m_COMPortSender->tech_read(2);
    if(len2 != 0)
    {
        ui->tech_error->setText(" ");
        result_tech = m_COMPortSender->tech_read_buf(2,len2);
        QString buf;
        QStringList list3 = result_tech.split(" ");
        int s=list3.size ();
        for(int i = 0; i < s - 1; i++)
        {
            if(list3[i]=="em")
            {
                ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                ui->tech_error->setText(" Буфер пуст");
            }

            if(list3[i]=="uu")
            {
                ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                ui->tech_error->setText(" Буфер переполнен");
            }
            else
            {
                buf+="0x";
                char hex = list3[i].toInt();
                buf+=QString::number(hex,16);
                buf+="\n";
                ui->tech_buf->setText(buf);
            }
        }
    }
}
void MainWindow::statusM()
{
    QString res;
    on_pushButton_ctm_ch_15_clicked();
    res += m_COMPortSender->req_stm();
    res += m_COMPortSender->req_tech();
    connect(this, SIGNAL( OTD_req()), myOTD, SLOT(OTD_req()));
    emit OTD_req();
    disconnect(this, SIGNAL( OTD_req()), myOTD, SLOT(OTD_req()));
    ui->error_mod->setText (res);

}
void MainWindow::status_OTD (QString data)
{
    if(data != "")
    {
        ui->error_mod->setText (data);
    }
}

void MainWindow::plot_point()
{
    double a = 0; //Начало интервала, где рисуем график по оси Ox
    double b =  k; //Конец интервала, где рисуем график по оси Ox
    int N=k+1; //Вычисляем количество точек, которые будем отрисовывать
    QVector<double> x(N), y(N),z(N); //Массивы координат точек
    double tra,tra1;
    int i=0;
    for (double X = 0; X < b; X++)//Пробегаем по всем точкам
    {
        tra=dat[i];
        tra1=dat1[i];
        x[i] = X*2;
        y[i] = tra;
        z[i] = tra1;
        i++;
    }
    ui->widget->clearGraphs();//Если нужно, то очищаем все графики
    ui->widget->addGraph();
    ui->widget->graph(0)->setData(x, y);
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setData(x, z);
    ui->widget->graph(0)->setName("I(БУП НА),A ");
    ui->widget->graph(1)->setName("I(ПНА),A ");
    ui->widget->xAxis->setTickLabelFont(QFont(QFont().family(), 10));
    ui->widget->yAxis->setTickLabelFont(QFont(QFont().family(), 10));
    ui->widget->xAxis->setLabelFont(QFont(QFont().family(), 10));
    ui->widget->yAxis->setLabelFont(QFont(QFont().family(), 10));
    ui->widget->xAxis->setLabel("Время, с");
    ui->widget->yAxis->setLabel(" I изм, А");
    ui->widget->xAxis->setRange(a, 1000);//Для оси Ox
    ui->widget->yAxis->setRange(-0.5, 10.5);//Для оси Oy
    ui->widget->legend->setVisible(true);
    ui->widget->replot();
}

void MainWindow::on_pushButton_U1_clicked()
{
    QString S1 = ui->setU1->text();
    double u1 = S1.toDouble();
    QString S3 = ui->setlimU1->text();
    double ul1 = S3.toDouble();
    QString S4 = ui->setlimI1->text();
    double Il1 = S4.toDouble();
    if(u1 > ul1)
    {
        u1 = ul1;
    }

    ui->setU1->setText(QString::number(u1));
    m_COMPortSender->setMaxVoltageAndCurrent(COMPortSender::POW_ANT_DRV_CTRL, ul1, Il1);
    m_COMPortSender->setVoltageAndCurrent(COMPortSender::POW_ANT_DRV_CTRL, u1);
}

void MainWindow::on_pushButton_U2_clicked()
{
    QString S1 = ui->setU2->text();
    double u1 = S1.toDouble();
    QString S3 = ui->setlimU2->text();
    double ul1 = S3.toDouble();
    QString S4 = ui->setlimI2->text();
    double Il1 = S4.toDouble();
    if(u1 > ul1)
    {
        u1 = ul1;
    }

    ui->setU2->setText(QString::number(u1));
    m_COMPortSender->setMaxVoltageAndCurrent(COMPortSender::POW_ANT_DRV, ul1, Il1);
    m_COMPortSender->setVoltageAndCurrent(COMPortSender::POW_ANT_DRV, u1);
}

void MainWindow::on_pushButton_2_clicked()
{
    m_COMPortSender->resetError(COMPortSender::POW_ANT_DRV);
    ui->err1->setText(" ");
}

void MainWindow::on_pushButton_3_clicked()
{
    m_COMPortSender->resetError(COMPortSender::POW_ANT_DRV_CTRL);
    ui->err2->setText(" ");
}

void MainWindow::on_pushButton_4_clicked()
{
    if(m_COMPortSender->setPowerChannelState(1, COMPortSender::POWER_ON) == 1 && flag_con1 == 0)
    {
        flag_con1 = 1;
        ui->pushButton_4->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else if(m_COMPortSender->setPowerChannelState(1, COMPortSender::POWER_OFF) == 1 && flag_con1 == 1)
    {
        flag_con1 = 0;
        ui->pushButton_4->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }
}
void MainWindow::on_pushButton_7_clicked()
{
    if(m_COMPortSender->setPowerChannelState(2, COMPortSender::POWER_ON) == 1 && flag_con3 == 0)
    {
        flag_con3 = 1;
        ui->pushButton_7->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else if(m_COMPortSender->setPowerChannelState(2, COMPortSender::POWER_OFF) == 1 && flag_con3 == 1)
    {
        flag_con3 = 0;
        ui->pushButton_7->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }
}
void MainWindow::on_pushButton_5_clicked()
{
    if(m_COMPortSender->setPowerChannelState(4, COMPortSender::POWER_ON) == 1 && flag_con2 == 0)
    {
        flag_con2 = 1;
        ui->pushButton_5->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else if(m_COMPortSender->setPowerChannelState(4, COMPortSender::POWER_OFF) == 1 && flag_con2 == 1)
    {
        flag_con2 = 0;
        ui->pushButton_5->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }
}

void MainWindow::on_pushButton_8_clicked()
{
    if(m_COMPortSender->setPowerChannelState(5, COMPortSender::POWER_ON) == 1 && flag_con4 == 0)
    {
        flag_con4 = 1;
        ui->pushButton_8->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else if(m_COMPortSender->setPowerChannelState(5, COMPortSender::POWER_OFF) == 1 && flag_con4 == 1)
    {
        flag_con4 = 0;
        ui->pushButton_8->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }
}

void MainWindow::on_pushButton_10_clicked()
{
    if(m_COMPortSender->setPowerChannelState(6, COMPortSender::POWER_ON) == 1 && flag_con5 == 0)
    {
        flag_con5 = 1;
        ui->pushButton_10->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else if(m_COMPortSender->setPowerChannelState(6, COMPortSender::POWER_OFF) == 1 && flag_con5 == 1)
    {
        flag_con5 = 0;
        ui->pushButton_10->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }
}

void MainWindow::on_pushButton_ctm_ch_0_clicked()
{
    double res = m_COMPortSender->stm_data_ch(0)/10000;
    ui->lineEdit_ctm_ch_0->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_1_clicked()
{
    double res = m_COMPortSender->stm_data_ch(1)/10000;
    ui->lineEdit_ctm_ch_1->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_2_clicked()
{
    double res = m_COMPortSender->stm_data_ch(2)/10000;
    ui->lineEdit_ctm_ch_2->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_3_clicked()
{
    double res = m_COMPortSender->stm_data_ch(3)/10000;
    ui->lineEdit_ctm_ch_3->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_4_clicked()
{
    double res = m_COMPortSender->stm_data_ch(4)/10000;
    ui->lineEdit_ctm_ch_4->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_5_clicked()
{
    double res = m_COMPortSender->stm_data_ch(5)/10000;
    ui->lineEdit_ctm_ch_5->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_6_clicked()
{
    double res = m_COMPortSender->stm_data_ch(6)/10000;
    ui->lineEdit_ctm_ch_6->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_7_clicked()
{
    double res = m_COMPortSender->stm_data_ch(7)/10000;
    ui->lineEdit_ctm_ch_7->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_8_clicked()
{
    double res = m_COMPortSender->stm_data_ch(8)/10000;
    ui->lineEdit_ctm_ch_8->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_9_clicked()
{
    double res = m_COMPortSender->stm_data_ch(9)/10000;
    ui->lineEdit_ctm_ch_9->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_10_clicked()
{
    double res = m_COMPortSender->stm_data_ch(10)/10000;
    ui->lineEdit_ctm_ch_10->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_11_clicked()
{
    double res = m_COMPortSender->stm_data_ch(11)/10000;
    ui->lineEdit_ctm_ch_11->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_12_clicked()
{
    double res = m_COMPortSender->stm_data_ch(12)/10000;
    ui->lineEdit_ctm_ch_12->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_13_clicked()
{
    double res = m_COMPortSender->stm_data_ch(13)/10000;
    ui->lineEdit_ctm_ch_13->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_14_clicked()
{
    double res = m_COMPortSender->stm_data_ch(14)/10000;
    ui->lineEdit_ctm_ch_14->setText(QString::number(res));
}

void MainWindow::on_pushButton_ctm_ch_15_clicked()
{
    double res = m_COMPortSender->stm_data_ch(15)/10000;
    if(res >= 0.5)
    {
        ui->pushButton_ctm_ch_15->setText ("Разъединена");
        ui->pushButton_ctm_ch_15->setStyleSheet(QString::fromUtf8("background-color: rgb(250, 24, 0);"));
    }
    else if(res >= 0 && res < 0.51 && m_COMPortSender->req_stm()=="")
    {
        ui->pushButton_ctm_ch_15->setText ("Соединена");
        ui->pushButton_ctm_ch_15->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 230, 0);"));
    }
}

void MainWindow::on_pushButton_check_fuse_1_clicked()
{
    int cf = m_COMPortSender->stm_check_fuse(1);
    if (cf==0 )
        ui->lineEdit_fuse_1->setText(" Исправен");
    else if ( cf==1 )
        ui->lineEdit_fuse_1->setText(" Неисправен");
    else if ( cf==2 )
        ui->lineEdit_fuse_1->setText(" Ошибка");
}

void MainWindow::on_pushButton_check_fuse_2_clicked()
{
    int cf = m_COMPortSender->stm_check_fuse(2);
    if (cf ==0)
        ui->lineEdit_fuse_2->setText(" Исправен");
    else if ( cf==1 )
        ui->lineEdit_fuse_2->setText(" Неисправен");
    else if ( cf==2 )
        ui->lineEdit_fuse_2->setText(" Ошибка");
}

void MainWindow::on_pushButton_tech_fd_clicked()
{
    if(m_COMPortSender->tech_send(36,0,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при установке режима работы!");
    }
}

void MainWindow::on_pushButton_tech_hd_clicked()
{
    if(m_COMPortSender->tech_send(36,1,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при установке режима работы!");
    }
}

void MainWindow::on_tech_set_speed_clicked()
{
    int sp= ui->baudRateBox_tech->currentIndex();
    if(m_COMPortSender->tech_send(37,sp,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при установке скорости работы!");
    }
}

void MainWindow::on_pushButton_send_tech_clicked()
{
    QString S1;
    S1 = ui->str_tech->text();
    QStringList list1 = S1.split(" ");
    int s=list1.size ();
    int cou=0;
    for(int i=0;i<s;i++)
    {
        if(list1[i]=="")
        {
            cou=cou+1;
        }
    }
    list1.removeAll ("");
    s=s-cou;
    int x =( s >> 8 ) & 0xFF;
    int y = s & 0xFF;
    m_COMPortSender->tech_send(22,x,y) ;
    bool ok;
    int ersend;
    int hex;
    for (int i = 0;  i< s; i++)
    {
        if (list1[i]!="")
        {
            hex = list1[i].toInt(&ok, 16);
            if(ok==1 && hex<256)
            {
                ui->tech_error->setText("");
                ersend = m_COMPortSender->tech_send(23,hex,0);
                if(ersend==2)
                {
                    ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                    ui->tech_error->setText(" Oшибка при формировании посылки данных!");
                }
            }
            else
            {
                i=s;
                ersend=2;
                ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                ui->tech_error->setText(" Oшибка во входящих данных!");
            }
        }
    }

    if(ersend==1)
    {
        ersend = m_COMPortSender->tech_send(24,0,0);
        if(ersend!=1)
        {
            ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
            ui->tech_error->setText(" Oшибка при передаче посылки данных!");
        }
    }
}

void MainWindow::on_tech_clear_out_3_clicked()
{
    if(m_COMPortSender->tech_send(27,1,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при очистке буфера!");
    }
}

void MainWindow::on_tech_clear_in_3_clicked()
{
    if(m_COMPortSender->tech_send(27,2,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при очистке буфера!");
    }
}

void MainWindow::on_tech_clear_buf_3_clicked()
{
    if(m_COMPortSender->tech_send(27,3,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при очистке буферов!");
    }
}

void MainWindow::on_tech_clear_out_4_clicked()
{
    if(m_COMPortSender->tech_send(21,1,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при очистке буфера!");
    }
}

void MainWindow::on_tech_clear_in_4_clicked()
{
    if(m_COMPortSender->tech_send(21,2,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при очистке буфера!");
    }
}

void MainWindow::on_tech_clear_buf_4_clicked()
{
    if(m_COMPortSender->tech_send(21,3,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при очистке буферов!");
    }
}

void MainWindow::on_pushButton_send_tech_2_clicked()
{
    QString S1;
    S1 = ui->str_tech_2->text();
    QStringList list1 = S1.split(" ");
    int s=list1.size ();
    int cou=0;
    for(int i=0;i<s;i++)
    {
        if(list1[i]=="")
        {
            cou=cou+1;
        }
    }
    list1.removeAll ("");
    s=s-cou;
    int x =( s >> 8 ) & 0xFF;
    int y = s & 0xFF;
    m_COMPortSender->tech_send(16,x,y) ;
    bool ok;
    int ersend;
    int hex;
    for (int i = 0;  i< s; i++)
    {
        if (list1[i]!="")
        {
            hex = list1[i].toInt(&ok, 16);
            if(ok==1 && hex<256)
            {
                ui->tech_error->setText("");
                ersend = m_COMPortSender->tech_send(17,hex,0);
                if(ersend==2)
                {
                    ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                    ui->tech_error->setText(" Oшибка при формировании посылки данных!");
                }
            }
            else
            {
                i=s;
                ersend=2;
                ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                ui->tech_error->setText(" Oшибка во входящих данных!");
            }
        }
    }

    if(ersend==1)
    {
        ersend = m_COMPortSender->tech_send(18,0,0);
        if(ersend!=1)
        {
            ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
            ui->tech_error->setText(" Oшибка при передаче посылки данных!");
        }
    }
}

void MainWindow::on_res_err_stm_clicked()
{
    if(m_COMPortSender->res_err_stm() != 1)
    {
        ui->error_mod->setText(" Не удалось сбросить ошибку!");
    }
}

void MainWindow::on_res_err_otd_clicked()
{
    connect(this, SIGNAL( OTD_err()), myOTD, SLOT(err_res_OTD()));
    emit OTD_err();
    disconnect(this, SIGNAL( OTD_res()), myOTD, SLOT(err_res_OTD()));
}

void MainWindow::OTD_err_res(int x)
{
    if(x != 1)
    {
        ui->error_mod->setText(" Не удалось сбросить ошибку!");
    }
}

void MainWindow::on_res_err_tech_clicked()
{
    if(m_COMPortSender->res_err_tech()!=1)
        ui->error_mod->setText(" Не удалось сбросить ошибку!");
}

void MainWindow::on_pushButton_res_stm_clicked()
{
    if(m_COMPortSender->res_stm()!=1)
        ui->error_mod->setText(" Не удалось провести перезагрузку!");
}

void MainWindow::on_pushButton_res_tech_clicked()
{
    if(m_COMPortSender->res_tech()!=1)
        ui->error_mod->setText(" Не удалось провести перезагрузку!");
}
void MainWindow::on_pushButton_res_otd_clicked()
{
    connect(this, SIGNAL( OTD_res()), myOTD, SLOT(res_OTD()));
    emit OTD_res();
    disconnect(this, SIGNAL( OTD_res()), myOTD, SLOT(res_OTD()));
}
void MainWindow::OTD_res_st(int x)
{
    if(x!=1)
        ui->error_mod->setText(" Не удалось провести перезагрузку!");
}
void MainWindow::OTD_id()
{
    ui->error_mod->setText(" Модуль ОТД установлен не в свой слот!");
}
void MainWindow::on_pushButton_6_clicked()
{
    double v = m_COMPortSender->fw_stm();
    if(v==2)
        ui->error_mod->setText(" Не удалось узнать версию прошивки!");
    else
        ui->lineEdit->setText (QString::number(v/10));
}

void MainWindow::on_pushButton_9_clicked()
{
    double v = m_COMPortSender->fw_tech();
    if(v==2)
        ui->error_mod->setText(" Не удалось узнать версию прошивки!");
    else
        ui->lineEdit_4->setText (QString::number(v/10));
}

void MainWindow::on_pushButton_13_clicked()
{
    connect(this, SIGNAL( OTD_sfw()), myOTD, SLOT(OTD_fw()));
    emit OTD_sfw();
    disconnect(this, SIGNAL( OTD_sfw()), myOTD, SLOT(OTD_fw()));
}

void MainWindow::OTD_fw(double x)
{
    if(x==2)ui->error_mod->setText(" Не удалось узнать версию прошивки!");
    else ui->lineEdit_5->setText (QString::number(x/10));
}

void MainWindow::OTDPTdata(double x,double y)
{
    x=x/100;
    y=y/100;
    ui->OTDerror->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
    if(x==-256)ui->OTDerror->setText("Ошибка измерения датчика");
    if(y==-256)ui->OTDerror->setText("Ошибка измерения датчика");
    if(x>1790)ui->OTDerror->setText("Ошибка обращения к модулю датчика");
    if(y>1790)ui->OTDerror->setText("Ошибка обращения к модулю датчика");
    ui->OTDPT1->setText(QString::number(x));
    ui->OTDPT2->setText(QString::number(y));
}

void MainWindow::on_OTDPT_clicked()
{
    connect(this, SIGNAL( OTD1()), myOTD, SLOT(OTDPT()));
    emit OTD1();
    disconnect(this, SIGNAL( OTD1()), myOTD, SLOT(OTDPT()));
}

void MainWindow::OTDtemd(QString data)
{
    ui->OTDtd->setText(data);
}

void MainWindow::on_OTD_reset_1_clicked()
{
    //myOTD->moveToThread(threadOTD);
    connect(this, SIGNAL( OTD_reset1()), myOTD, SLOT(OTDres1()));
    emit OTD_reset1();
    disconnect(this, SIGNAL( OTD_reset1()), myOTD, SLOT(OTDres1()));
}

void MainWindow::on_OTD_reset_2_clicked()
{
    connect(this, SIGNAL( OTD_reset2()), myOTD, SLOT(OTDres2()));
    emit OTD_reset2();
    disconnect(this, SIGNAL( OTD_reset2()), myOTD, SLOT(OTDres2()));
}

void MainWindow::OTDerror(QString err)
{
    ui->OTDerror->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
    ui->OTDerror->setText(err);
}

void MainWindow::on_OTD_meas_1_clicked()
{
    connect(this, SIGNAL( OTD_meas1()), myOTD, SLOT(OTDmeas1()));
    emit OTD_meas1();
    disconnect(this, SIGNAL( OTD_meas1()), myOTD, SLOT(OTDmeas1()));
}

void MainWindow::on_OTD_meas_2_clicked()
{
    connect(this, SIGNAL( OTD_meas2()), myOTD, SLOT(OTDmeas2()));
    emit OTD_meas2();
    disconnect(this, SIGNAL( OTD_meas2()), myOTD, SLOT(OTDmeas2()));
}

void MainWindow::OTDtm1(QString temp)
{
    ui->OTDtm1->setText(temp);
}

void MainWindow::OTDtm2(QString temp)
{
    ui->OTDtm2->setText(temp);
}

void MainWindow::on_OTD_nd_clicked()
{
    //myOTD->moveToThread(threadOTD);
    connect(this, SIGNAL( OTD_nd()), myOTD, SLOT(OTDtemper()));
    emit OTD_nd();
    disconnect(this, SIGNAL( OTD_nd()), myOTD, SLOT(OTDtemper()));
}

void MainWindow::on_pow_DY_osn_clicked()
{
    QString S1 = ui->lineEdit_Addr_2->text();
    int u1 = S1.toInt();
    connect(this, SIGNAL( MKO_DY(int, int)), myMKO, SLOT(pow_DY(int, int)));
    emit MKO_DY(0,u1);
    disconnect(this, SIGNAL( MKO_DY(int, int)), myMKO, SLOT(pow_DY(int, int)));
}

void MainWindow::on_pow_DY_rez_clicked()
{
    QString S2 = ui->lineEdit_Addr_3->text();
    int u2 = S2.toInt();
    connect(this, SIGNAL( MKO_DY(int, int)), myMKO, SLOT(pow_DY(int, int)));
    emit MKO_DY(1,u2);
    disconnect(this, SIGNAL( MKO_DY(int, int)), myMKO, SLOT(pow_DY(int, int)));
}

void MainWindow::on_MKO_osn_clicked()
{
    if(flag_mko_osn==0){
        flag_mko_osn=1;
        ui->MKO_osn->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else if(flag_mko_osn==1){
        flag_mko_osn=0;
        ui->MKO_osn->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }
    connect(this, SIGNAL( MKO_ch(int)), myMKO, SLOT(MKO_chan(int)));
    emit MKO_ch(flag_mko_osn+flag_mko_rez);
    disconnect(this, SIGNAL( MKO_ch(int)), myMKO, SLOT(MKO_chan(int)));
}

void MainWindow::on_MKO_rez_clicked()
{
    if(flag_mko_rez==0){
        flag_mko_rez=2;
        ui->MKO_rez->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else if(flag_mko_rez==2){
        flag_mko_rez=0;
        ui->MKO_rez->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }
    connect(this, SIGNAL( MKO_ch(int)), myMKO, SLOT(MKO_chan(int)));
    emit MKO_ch(flag_mko_osn+flag_mko_rez);
    disconnect(this, SIGNAL( MKO_ch(int)), myMKO, SLOT(MKO_chan(int)));
}

void MainWindow::on_MKO_test_clicked()
{
    QString S1 = ui->lineEdit_Addr_2->text();
    int u1 = S1.toInt();
    QString S2 = ui->lineEdit_Addr_3->text();
    int u2 = S2.toInt();
    connect(this, SIGNAL( MKO_ts(int,int,int)), myMKO, SLOT(MKO_start_test(int,int,int)));
    emit MKO_ts(flag_mko_osn+flag_mko_rez,u1,u2);
    disconnect(this, SIGNAL( MKO_ts(int,int,int)), myMKO, SLOT(MKO_start_test(int,int,int)));
}

void MainWindow::on_pushButton_11_clicked()
{
    QString S1;
    S1 += ui->MKO_cd_0->text()+" ";
    S1 += ui->MKO_cd_1->text()+" ";
    S1 += ui->MKO_cd_3->text()+" ";
    S1 += ui->MKO_cd_4->text()+" ";
    S1 += ui->MKO_cd_5->text()+" ";
    S1 += ui->MKO_cd_6->text()+" ";
    S1 += ui->MKO_cd_8->text()+" ";
    S1 += ui->MKO_cd_9->text();
    QString S3 = ui->lineEdit_Addr_2->text();
    int u1 = S3.toInt();
    QString S2 = ui->lineEdit_Addr_3->text();
    int u2 = S2.toInt();
    connect(this, SIGNAL( MKO_cm(int,QString, int,int)), myMKO, SLOT(MKO_tr_cm(int,QString,int,int)));
    emit MKO_cm(flag_mko_osn+flag_mko_rez,S1,u1,u2);
    disconnect(this, SIGNAL( MKO_cm(int,QString,int,int)), myMKO, SLOT(MKO_tr_cm(int,QString,int,int)));
}

void MainWindow::on_pushButton_12_clicked()
{
    QString S1 = ui->lineEdit_Addr_2->text();
    int u1 = S1.toInt();
    QString S2 = ui->lineEdit_Addr_3->text();
    int u2 = S2.toInt();
    connect(this, SIGNAL( MKO_cm_r(int,int,int)), myMKO, SLOT(MKO_rc_cm(int,int,int)));
    emit MKO_cm_r(flag_mko_osn+flag_mko_rez,u1,u2);
    disconnect(this, SIGNAL( MKO_cm_r(int,int,int)), myMKO, SLOT(MKO_rc_cm(int,int,int)));
}


void MainWindow::MKO_change_ch(int x, int y)
{
    m_COMPortSender->stm_on_mko(x,y);
}

void MainWindow::on_MKO_avt_clicked()
{
    QString S1 = ui->lineEdit_Addr_2->text();
    int u1 = S1.toInt();
    QString S2 = ui->lineEdit_Addr_3->text();
    int u2 = S2.toInt();
    QString S3 = ui->lineEdit_period->text();
    int u3 = S3.toInt();
    if(flag_mko_auto==0)
    {
        flag_mko_auto=1;
        ui->MKO_avt->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
        connect(this, SIGNAL( MKO_auto(int,int,int,int)), myMKO, SLOT(MKO_avt(int,int,int,int)));
        emit MKO_auto(flag_mko_auto,u3,u1,u2);
        disconnect(this, SIGNAL( MKO_auto(int,int,int,int)), myMKO, SLOT(MKO_avt(int,int,int,int)));
    }
    else if(flag_mko_auto==1)
    {
        flag_mko_auto=0;
        ui->MKO_avt->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
        connect(this, SIGNAL( MKO_auto(int,int,int,int)), myMKO, SLOT(MKO_avt(int,int,int,int)));
        emit MKO_auto(flag_mko_auto,u3,u1,u2);
        disconnect(this, SIGNAL( MKO_auto(int,int,int,int)), myMKO, SLOT(MKO_avt(int,int,int,int)));
    }
}

void MainWindow::on_OTD_avt_2_clicked()
{
    QString S3 = ui->lineEdit_period_OTD->text();
    int u3 = S3.toInt();
    if(flag_otd_auto==0)
    {
        flag_otd_auto=1;
        ui->OTD_avt_2->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
        connect(this, SIGNAL( OTD_auto(int,int)), myOTD, SLOT(OTD_avt(int,int)));
        emit OTD_auto(flag_otd_auto,u3*1000);
        disconnect(this, SIGNAL( OTD_auto(int,int)), myOTD, SLOT(OTD_avt(int,int)));
    }
    else if(flag_otd_auto==1)
    {
        flag_otd_auto=0;
        ui->OTD_avt_2->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
        connect(this, SIGNAL( OTD_auto(int,int)), myOTD, SLOT(OTD_avt(int,int)));
        emit OTD_auto(flag_otd_auto,u3);
        disconnect(this, SIGNAL( OTD_auto(int,int)), myOTD, SLOT(OTD_avt(int,int)));
    }
}










