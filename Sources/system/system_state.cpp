#include "Headers/system/system_state.h"
#include "Headers/system/myclass.h"
#include "Headers/system/OTD.h"
#include "Headers/system/modules/module_mko.h"
#include "Headers/module_commands.h"

#include <QtSerialPort>
#include <windows.h>
#include "qapplication.h"


namespace
{
    static const uint8_t OTD_DEFAULT_ADDR = 0x44;
    static const uint8_t STM_DEFAULT_ADDR = 0x22;
    static const uint8_t TECH_DEFAULT_ADDR = 0x56;

    static const uint8_t MAX_VOLTAGE = 42; // volts
    static const uint8_t MAX_CURRENT = 10; // ampers
    static const uint8_t MAX_POWER = 155; // watts actually 160, but for safety purposes reduced to 155
}

SystemState::SystemState(QObject* parent):
    VariableController(parent),
    m_mko_kits(ModuleMKO::NO_KIT)
{
}

SystemState::~SystemState()
{
    foreach (ModuleInfo info, m_modules)
    {
        if (info.port->isOpen())
        {
            info.port->close();
        }
    }
}

void SystemState::init()
{
    return;

    // 1. Create COM-ports
    // 2. Send command "Get module address" (current and default) to each of them
    // 3. Depending on list, create modules and put port pointers to them
    // 4. Call "init" of the each module

    /*
    QSerialPort *port = createPort();

    if (port->isOpen())
    {
        int i = 0;
    }
    else
    {
        qDebug("No QSerialPort created"); // TODO critical error
    }
    */

    mThreadOTD = new QThread(this);
    mOTD = new OTD("B", this);

    mThreadMKO = new QThread(this);
    mMKO = new ModuleMKO(this);

    // TODO: The order of ports creation possibly important!
    {
        ModuleInfo info;
        info.port = createPort("6");
        info.state = true;
        info.address = 0xFF;
        m_modules[POW_ANT_DRV] = info;
    }

    {
        ModuleInfo info;
        info.port = createPort("5");
        info.state = true;
        info.address = 0xFF;
        m_modules[POW_ANT_DRV_CTRL] = info;
    }

    {
        ModuleInfo info;
        info.port = createPort("4");
        info.state = true;
        info.address = STM_DEFAULT_ADDR;
        m_modules[STM] = info;
    }

    {
        ModuleInfo info;
        info.port = createPort("8");
        info.state = true;
        info.address = TECH_DEFAULT_ADDR;
        m_modules[TECH] = info;
    }

    startPower();

    stm_on_mko(1,0);
    stm_on_mko(2,0);

    mMKO->moveToThread(mThreadMKO);
    connect(mMKO, SIGNAL(test_MKO(int)), this, SLOT(simpltst1(int)));
    connect(mMKO, SIGNAL(MKO_CTM(int, int)), this, SLOT(MKO_change_ch(int, int)));
    connect(mMKO, SIGNAL(start_MKO(QString)), this, SLOT(MKO_data(QString)));
    connect(mMKO, SIGNAL(data_MKO(QString)), this, SLOT(MKO_cm_data(QString)));
    mThreadMKO->start();

    mOTD->moveToThread(mThreadOTD);
    connect(mOTD, SIGNAL(start_OTDPT(double,double)), this, SLOT(OTDPTdata(double,double)));
    connect(mOTD, SIGNAL(temp_OTD(QString)), this, SLOT(OTDtemd(QString)));
    connect(mOTD, SIGNAL(OTD_res(int)), this, SLOT(OTD_res_st(int)));
    connect(mOTD, SIGNAL(OTD_reqr(QString)), this, SLOT(status_OTD(QString)));
    connect(mOTD, SIGNAL(OTD_err_res(int)), this, SLOT(OTD_err_res(int)));
    connect(mOTD, SIGNAL(OTD_id1()), this, SLOT(OTD_id()));
    connect(mOTD, SIGNAL(OTD_vfw(double)), this, SLOT(OTD_fw(double)));
    connect(mOTD, SIGNAL(err_OTD(QString)), this, SLOT(OTDerror(QString)));
    connect(mOTD, SIGNAL(tm_OTD1(QString)), this, SLOT(OTDtm1(QString)));
    connect(mOTD, SIGNAL(tm_OTD2(QString)), this, SLOT(OTDtm2(QString)));
    connect(mThreadOTD, SIGNAL(started()), mOTD, SLOT(COMConnectorOTD()));
    mThreadOTD->start();

    QThread *thread = new QThread;
    MyClass *my = new MyClass("B");
    my->moveToThread(thread);
    connect(my, SIGNAL(send()), this, SLOT(paintvalue()));
    connect(my, SIGNAL(send3()), this, SLOT(statusRS()));
    connect(my, SIGNAL(send4()), this, SLOT(statusCAN()));
    connect(my, SIGNAL(send5()), this, SLOT(statusM()));
    connect(thread, SIGNAL(started()), my, SLOT(doWork()));
    thread->start();
    //startpower(); // Ivan Semenchuk: what for to do it twice?

    QString error_m;
    if(id_stm() != 1)
    {
        error_m += " Модуль СТМ установлен не в свой слот!";
    }

    if(id_tech() != 1)
    {
        error_m += " Модуль ТЕХНОЛОГИЧЕСКИЙ установлен не в свой слот!";
    }

    //ui->error_mod->setText(error_m);
}

int SystemState::simpltst1(int z)
{
    if(z == 0)
    {
        //ui->label_MKO->setText("Тест пройден успешно!\n");
    }
    else
    {
        QString TestOutStr = "Ошибка! Тест провален с ";
        TestOutStr += QString::number(z);
        TestOutStr += " ошибками. Перезагрузите программу!\n";
        //ui->label_MKO->setText(TestOutStr);
    }

    return z ;
}

void SystemState::MKO_data(QString data)
{
    //ui->label_MKO->setText(data);
}

void SystemState::MKO_cm_data(QString data)
{
    QString ncd, zcd, cdh, cdd;

    QStringList list1 = data.split(" ");
    QString list2[42];
    list2[0] +="Текущий режим работы ось ψ\n(от -32767 до +32767)";
    list2[1] +="Заданное положение в шагах ось ψ\n( от -214748648 до +214748648)";
    list2[2] +="Заданное положение в шагах ось ψ\n( от -214748648 до +214748648)";
    list2[3] +="Текущая скорость ось ψ\n ( от -32767 до +32767)";
    list2[4] +="Текущий ток ось ψ\n ( от -511 до +511)";
    list2[5] +="Текущий режим работы ось υ\n(от -32767 до +32767)";
    list2[6] +="Заданное положение в шагах ось υ\n( от -214748648 до +214748648) ";
    list2[7] +="Заданное положение в шагах ось υ\n( от -214748648 до +214748648)";
    list2[8] +="Текущая скорость ось υ\n ( от -32767 до +32767)";
    list2[9] +="Текущий ток ось υ\n ( от -511 до +511)";
    list2[10] +="Информация датчика угла по ψ\n (0…180град)";
    list2[11] +="Информация датчика угла по υ\n (0…180град) ";
    list2[12] +="Не используется";
    list2[13] +="Не используется";
    list2[14] +="Не используется";
    list2[15] +="Не используется";
    list2[16] +="Не используется";
    list2[17] +="Не используется";
    list2[18] +="Признак ошибки контрольной\n суммы принятого массива";
    list2[19] +="Информация термодатчика в град.\n (0xFFFF-датчика нет)";
    list2[20] +="Контрольная сумма";

    //delete_table_MKO();

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

            //add_string_table_MKO(ct, ncd, zcd, cdh, cdd);

            ct++;
            ncd = "";
            zcd = "";
            cdh = "";
            cdd = "";
        }
    }

    if(m_mko_kits == ModuleMKO::ALL_KITS)
    {
        if (list1[0] != "")
        {
            //add_string_table_MKO(ct, "----------- ", "'РЕЗЕРВНЫЙ ПОЛУКОМПЛЕКТ'", "---------- ", "----------- ");
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

                //add_string_table_MKO(ct, ncd, zcd, cdh, cdd);
                ct++;
                ncd = "";
                zcd = "";
                cdh = "";
                cdd = "";
            }
        }
    }
}

void SystemState::on_pushButton_start_com6_clicked()
{
    if(m_flag_rem1 == 0)
    {
        m_flag_rem1 = 1;
        setPowerState(POW_ANT_DRV_CTRL, POWER_ON);
    }
    else
    {
        m_flag_rem1 = 0;
        setPowerState(POW_ANT_DRV_CTRL, POWER_OFF);
    }
}

void SystemState::on_pushButton_start_com5_clicked()
{
    if(m_flag_rem2 == 0)
    {
        m_flag_rem2 = 1;
        setPowerState(POW_ANT_DRV, POWER_ON);
    }
    else
    {
        m_flag_rem2 = 0;
        setPowerState(POW_ANT_DRV, POWER_OFF);
    }
}

void SystemState::paintvalue()
{
    double u1, u2, i1, i2;
    uint8_t er1, er2;

    //ui->tech_error->setText("");
    getCurVoltageAndCurrent(POW_ANT_DRV, u1, i1, er1);
    getCurVoltageAndCurrent(POW_ANT_DRV_CTRL, u2, i2, er2);

    //ui->err1->setStyleSheet("font: 25 9pt GOST type A;" "color: black;");
    //ui->err2->setStyleSheet("font: 25 9pt GOST type A;" "color: black;");
    if(er1 != 0)
    {
        m_flag_rem1 = 0;
        //ui->pushButton_start_com6->setChecked(false);
    }

    if(er2 != 0)
    {
        m_flag_rem2 = 0;
        //ui->pushButton_start_com5->setChecked(false);
    }

    //if (er1 == 1) ui->err1->setText("Overvoltage protection!");
    //if (er1 == 2) ui->err1->setText("Overcurrent protection!");
    //if (er1 == 4) ui->err1->setText("Overpower protection!");
    //if (er1 == 8) ui->err1->setText("Overtemperature protection!");

    //if (er2 == 1) ui->err2->setText("Overvoltage protection!");
    //if (er2 == 2) ui->err2->setText("Overcurrent protection!");
    //if (er2 == 4) ui->err2->setText("Overpower protection!");
    //if (er2 == 8) ui->err2->setText("Overtemperature protection!");

    //ui->U1out->setText(QString::number(u1 / 100));
    //ui->U2out->setText(QString::number(u2 / 100));

    if(m_k > 500) // wtf?
    {
        m_k = 0;
    }

    m_dat[m_k] = i1 / 100;
    m_dat1[m_k] = i2 / 100;
    m_k++;
    plot_point();
}

void SystemState::statusRS()
{
    int len1 = tech_read(1);
    if(len1 != 0)
    {
        //ui->tech_error->setText(" ");
        QString result_tech = tech_read_buf(1, len1);
        QString buf;
        QStringList list2 = result_tech.split(" ");
        int s = list2.size();
        for(int i = 0; i < s - 1; i++)
        {
            if(list2[i] == "em")
            {
                //ui->tech_error->setText(" Буфер пуст");
            }

            if(list2[i] == "uu")
            {
                //ui->tech_error->setText(" Буфер переполнен");
            }
            else
            {
                buf += "0x";
                char hex = list2[i].toInt();
                buf += QString::number(hex,16);
                buf += "\n";
                //ui->tech_buf->setText(buf);
            }
        }
    }
}

void SystemState::statusCAN()
{
    int len2 = tech_read(2);
    if(len2 != 0)
    {
        //ui->tech_error->setText(" ");
        QString result_tech = tech_read_buf(2, len2);
        QString buf;
        QStringList list3 = result_tech.split(" ");
        int s = list3.size ();
        for(int i = 0; i < s - 1; i++)
        {
            if(list3[i]=="em")
            {
                //ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                //ui->tech_error->setText(" Буфер пуст");
            }

            if(list3[i]=="uu")
            {
                //ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                //ui->tech_error->setText(" Буфер переполнен");
            }
            else
            {
                buf += "0x";
                char hex = list3[i].toInt();
                buf += QString::number(hex, 16);
                buf += "\n";
                //ui->tech_buf->setText(buf);
            }
        }
    }
}

void SystemState::statusM()
{
    QString res;
    on_pushButton_ctm_ch_15_clicked();
    res += req_stm();
    res += req_tech();
    connect(this, SIGNAL(OTD_req()), mOTD, SLOT(OTD_req()));
    emit OTD_req();
    disconnect(this, SIGNAL(OTD_req()), mOTD, SLOT(OTD_req()));
    //ui->error_mod->setText(res);
}

void SystemState::status_OTD(QString data)
{
    if(data != "")
    {
        //ui->error_mod->setText (data);
    }
}

void SystemState::plot_point()
{
    double a = 0; //Начало интервала, где рисуем график по оси Ox
    double b = m_k; //Конец интервала, где рисуем график по оси Ox
    int N = m_k + 1; //Вычисляем количество точек, которые будем отрисовывать

    //Массивы координат точек
    QVector<double> x(N);
    QVector<double> y(N);
    QVector<double> z(N);
    double tra = 0;
    double tra1 = 0;
    int i = 0;

    for (double X = 0; X < b; X++)//Пробегаем по всем точкам
    {
        tra = m_dat[i];
        tra1 = m_dat1[i];
        x[i] = X * 2;
        y[i] = tra;
        z[i] = tra1;
        i++;
    }

    //ui->widget->clearGraphs();//Если нужно, то очищаем все графики
    //ui->widget->addGraph();
    //ui->widget->graph(0)->setData(x, y);
    //ui->widget->graph(0)->setPen(QPen(Qt::blue));
    //ui->widget->addGraph();
    //ui->widget->graph(1)->setPen(QPen(Qt::red));
    //ui->widget->graph(1)->setData(x, z);
    //ui->widget->graph(0)->setName("I(БУП НА),A ");
    //ui->widget->graph(1)->setName("I(ПНА),A ");
    //ui->widget->xAxis->setTickLabelFont(QFont(QFont().family(), 10));
    //ui->widget->yAxis->setTickLabelFont(QFont(QFont().family(), 10));
    //ui->widget->xAxis->setLabelFont(QFont(QFont().family(), 10));
    //ui->widget->yAxis->setLabelFont(QFont(QFont().family(), 10));
    //ui->widget->xAxis->setLabel("Время, с");
    //ui->widget->yAxis->setLabel(" I изм, А");
    //ui->widget->xAxis->setRange(a, 1000);//Для оси Ox
    //ui->widget->yAxis->setRange(-0.5, 10.5);//Для оси Oy
    //ui->widget->legend->setVisible(true);
    //ui->widget->replot();
}

void SystemState::on_pushButton_U1_clicked()
{
    /*
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

    setMaxVoltageAndCurrent(POW_ANT_DRV_CTRL, ul1, Il1);
    setVoltageAndCurrent(POW_ANT_DRV_CTRL, u1);
    */
}

void SystemState::on_pushButton_U2_clicked()
{
    /*
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
    setMaxVoltageAndCurrent(POW_ANT_DRV, ul1, Il1);
    setVoltageAndCurrent(POW_ANT_DRV, u1);
    */
}

void SystemState::on_pushButton_2_clicked()
{
    resetError(POW_ANT_DRV);
    //ui->err1->setText(" ");
}

void SystemState::on_pushButton_3_clicked()
{
    resetError(POW_ANT_DRV_CTRL);
    //ui->err2->setText(" ");
}

void SystemState::on_pushButton_4_clicked()
{
    if(setPowerChannelState(1, POWER_ON) == 1 && m_flag_con1 == 0)
    {
        m_flag_con1 = 1;
    }
    else if(setPowerChannelState(1, POWER_OFF) == 1 && m_flag_con1 == 1)
    {
        m_flag_con1 = 0;
    }
}
void SystemState::on_pushButton_7_clicked()
{
    if(setPowerChannelState(2, POWER_ON) == 1 && m_flag_con3 == 0)
    {
        m_flag_con3 = 1;
    }
    else if(setPowerChannelState(2, POWER_OFF) == 1 && m_flag_con3 == 1)
    {
        m_flag_con3 = 0;
    }
}

void SystemState::on_pushButton_5_clicked()
{
    if(setPowerChannelState(4, POWER_ON) == 1 && m_flag_con2 == 0)
    {
        m_flag_con2 = 1;
    }
    else if(setPowerChannelState(4, POWER_OFF) == 1 && m_flag_con2 == 1)
    {
        m_flag_con2 = 0;
    }
}

void SystemState::on_pushButton_8_clicked()
{
    if(setPowerChannelState(5, POWER_ON) == 1 && m_flag_con4 == 0)
    {
        m_flag_con4 = 1;
    }
    else if(setPowerChannelState(5, POWER_OFF) == 1 && m_flag_con4 == 1)
    {
        m_flag_con4 = 0;
    }
}

void SystemState::on_pushButton_10_clicked()
{
    if(setPowerChannelState(6, POWER_ON) == 1 && m_flag_con5 == 0)
    {
        m_flag_con5 = 1;
    }
    else if(setPowerChannelState(6, POWER_OFF) == 1 && m_flag_con5 == 1)
    {
        m_flag_con5 = 0;
    }
}

void SystemState::on_pushButton_ctm_ch_0_clicked()
{
    double res = stm_data_ch(0)/10000;
    //ui->lineEdit_ctm_ch_0->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_1_clicked()
{
    double res = stm_data_ch(1)/10000;
    //ui->lineEdit_ctm_ch_1->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_2_clicked()
{
    double res = stm_data_ch(2)/10000;
    //ui->lineEdit_ctm_ch_2->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_3_clicked()
{
    double res = stm_data_ch(3)/10000;
   // ui->lineEdit_ctm_ch_3->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_4_clicked()
{
    double res = stm_data_ch(4)/10000;
    //ui->lineEdit_ctm_ch_4->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_5_clicked()
{
    double res = stm_data_ch(5)/10000;
    //ui->lineEdit_ctm_ch_5->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_6_clicked()
{
    double res = stm_data_ch(6)/10000;
    //ui->lineEdit_ctm_ch_6->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_7_clicked()
{
    double res = stm_data_ch(7)/10000;
    //ui->lineEdit_ctm_ch_7->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_8_clicked()
{
    double res = stm_data_ch(8)/10000;
    //ui->lineEdit_ctm_ch_8->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_9_clicked()
{
    double res = stm_data_ch(9)/10000;
    //ui->lineEdit_ctm_ch_9->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_10_clicked()
{
    double res = stm_data_ch(10)/10000;
    //ui->lineEdit_ctm_ch_10->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_11_clicked()
{
    double res = stm_data_ch(11)/10000;
    //ui->lineEdit_ctm_ch_11->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_12_clicked()
{
    double res = stm_data_ch(12)/10000;
    //ui->lineEdit_ctm_ch_12->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_13_clicked()
{
    double res = stm_data_ch(13)/10000;
    //ui->lineEdit_ctm_ch_13->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_14_clicked()
{
    double res = stm_data_ch(14)/10000;
    //ui->lineEdit_ctm_ch_14->setText(QString::number(res));
}

void SystemState::on_pushButton_ctm_ch_15_clicked()
{
    double res = stm_data_ch(15)/10000;
    if(res >= 0.5)
    {
        //ui->pushButton_ctm_ch_15->setText ("Разъединена");
    }
    else if(res >= 0 && res < 0.51 && req_stm()=="")
    {
        //ui->pushButton_ctm_ch_15->setText ("Соединена");
    }
}

void SystemState::on_pushButton_check_fuse_1_clicked()
{
    /*
    int cf = stm_check_fuse(1);
    if (cf == 0)
        ui->lineEdit_fuse_1->setText(" Исправен");
    else if (cf == 1)
        ui->lineEdit_fuse_1->setText(" Неисправен");
    else if (cf == 2)
        ui->lineEdit_fuse_1->setText(" Ошибка");
        */
}

void SystemState::on_pushButton_check_fuse_2_clicked()
{
    /*
    int cf = stm_check_fuse(2);
    if (cf == 0)
        ui->lineEdit_fuse_2->setText(" Исправен");
    else if (cf == 1)
        ui->lineEdit_fuse_2->setText(" Неисправен");
    else if (cf == 2)
        ui->lineEdit_fuse_2->setText(" Ошибка");
        */
}

void SystemState::on_pushButton_tech_fd_clicked()
{
    /*
    if(tech_send(36, 0, 0) != 1)
    {
        ui->tech_error->setText(" Oшибка при установке режима работы!");
    }*/
}

void SystemState::on_pushButton_tech_hd_clicked()
{
    /*
    if(tech_send(36, 1, 0) != 1)
    {
        ui->tech_error->setText(" Oшибка при установке режима работы!");
    }
    */
}

void SystemState::on_tech_set_speed_clicked()
{
    /*
    int sp = ui->baudRateBox_tech->currentIndex();
    if(tech_send(37, sp, 0)!=1)
    {
        ui->tech_error->setText(" Oшибка при установке скорости работы!");
    }
    */
}

void SystemState::on_pushButton_send_tech_clicked()
{
    QString S1;
    //S1 = ui->str_tech->text();
    QStringList list1 = S1.split(" ");
    int s = list1.size ();
    int cou = 0;
    for(int i = 0; i < s; i++)
    {
        if(list1[i] == "")
        {
            cou = cou + 1;
        }
    }

    list1.removeAll ("");
    s = s - cou;
    int x =( s >> 8 ) & 0xFF;
    int y = s & 0xFF;
    tech_send(22,x,y) ;
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
                //ui->tech_error->setText("");
                ersend = tech_send(23,hex,0);
                if(ersend==2)
                {
                    //ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                    //ui->tech_error->setText(" Oшибка при формировании посылки данных!");
                }
            }
            else
            {
                i=s;
                ersend=2;
                //ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                //ui->tech_error->setText(" Oшибка во входящих данных!");
            }
        }
    }

    if(ersend==1)
    {
        ersend = tech_send(24,0,0);
        if(ersend!=1)
        {
            //ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
            //ui->tech_error->setText(" Oшибка при передаче посылки данных!");
        }
    }
}

void SystemState::on_tech_clear_out_3_clicked()
{
    /*
    if(tech_send(27, 1, 0) != 1)
    {
        ui->tech_error->setText(" Oшибка при очистке буфера!");
    }
    */
}

void SystemState::on_tech_clear_in_3_clicked()
{
    /*
    if(tech_send(27, 2, 0) != 1)
    {
        ui->tech_error->setText(" Oшибка при очистке буфера!");
    }
    */
}

void SystemState::on_tech_clear_buf_3_clicked()
{
    /*
    if(tech_send(27,3,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при очистке буферов!");
    }
    */
}

void SystemState::on_tech_clear_out_4_clicked()
{
    /*
    if(tech_send(21,1,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при очистке буфера!");
    }
    */
}

void SystemState::on_tech_clear_in_4_clicked()
{
    /*
    if(tech_send(21,2,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при очистке буфера!");
    }
    */
}

void SystemState::on_tech_clear_buf_4_clicked()
{
    /*
    if(tech_send(21,3,0)!=1)
    {
        ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
        ui->tech_error->setText(" Oшибка при очистке буферов!");
    }
    */
}

void SystemState::on_pushButton_send_tech_2_clicked()
{
    QString S1;
   // S1 = ui->str_tech_2->text();
    QStringList list1 = S1.split(" ");
    int s = list1.size ();
    int cou = 0;
    for(int i = 0; i < s; i++)
    {
        if(list1[i]=="")
        {
            cou = cou + 1;
        }
    }

    list1.removeAll ("");
    s = s - cou;
    int x =( s >> 8 ) & 0xFF;
    int y = s & 0xFF;
    tech_send(16, x, y);
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
                //ui->tech_error->setText("");
                ersend = tech_send(17,hex,0);
                if(ersend==2)
                {
                    //ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                    //ui->tech_error->setText(" Oшибка при формировании посылки данных!");
                }
            }
            else
            {
                i=s;
                ersend=2;
                //ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
                //ui->tech_error->setText(" Oшибка во входящих данных!");
            }
        }
    }

    if(ersend==1)
    {
        ersend = tech_send(18,0,0);
        if(ersend!=1)
        {
            //ui->tech_error->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
            //ui->tech_error->setText(" Oшибка при передаче посылки данных!");
        }
    }
}

void SystemState::on_res_err_stm_clicked()
{
    /*
    if(resetError(STM) != 1)
    {
        ui->error_mod->setText(" Не удалось сбросить ошибку!");
    }
    */
}

void SystemState::on_res_err_otd_clicked()
{
    connect(this, SIGNAL(OTD_err()), mOTD, SLOT(err_res_OTD()));
    emit OTD_err();
    disconnect(this, SIGNAL(OTD_res()), mOTD, SLOT(err_res_OTD()));
}

void SystemState::OTD_err_res(int x)
{
    /*
    if(x != 1)
    {
        ui->error_mod->setText(" Не удалось сбросить ошибку!");
    }
    */
}

void SystemState::on_res_err_tech_clicked()
{
    /*
    if(resetError(TECH) != 1)
    {
        ui->error_mod->setText(" Не удалось сбросить ошибку!");
    }
    */
}

void SystemState::on_pushButton_res_stm_clicked()
{
    /*
    if(softResetModule(STM) != 1)
    {
        ui->error_mod->setText(" Не удалось провести перезагрузку!");
    }
    */
}

void SystemState::on_pushButton_res_tech_clicked()
{
    /*
    if(softResetModule(TECH) != 1)
    {
        ui->error_mod->setText(" Не удалось провести перезагрузку!");
    }
    */
}

void SystemState::on_pushButton_res_otd_clicked()
{
    connect(this, SIGNAL(OTD_res()), mOTD, SLOT(res_OTD()));
    emit OTD_res();
    disconnect(this, SIGNAL(OTD_res()), mOTD, SLOT(res_OTD()));
}

void SystemState::OTD_res_st(int x)
{
    /*
    if(x!=1)
    {
        ui->error_mod->setText(" Не удалось провести перезагрузку!");
    }
    */
}

void SystemState::OTD_id()
{
    //ui->error_mod->setText(" Модуль ОТД установлен не в свой слот!");
}

void SystemState::on_pushButton_6_clicked()
{
    /*
    double v = getSoftwareVersion(STM);
    if(v == 2)
    {
        ui->error_mod->setText(" Не удалось узнать версию прошивки!");
    }
    else
    {
        ui->lineEdit->setText(QString::number(v / 10));
    }
    */
}

void SystemState::on_pushButton_9_clicked()
{
    /*
    double v = getSoftwareVersion(TECH);
    if(v == 2)
    {
        ui->error_mod->setText(" Не удалось узнать версию прошивки!");
    }
    else
    {
        ui->lineEdit_4->setText (QString::number(v / 10));
    }
    */
}

void SystemState::on_pushButton_13_clicked()
{
    connect(this, SIGNAL(OTD_sfw()), mOTD, SLOT(OTD_fw()));
    emit OTD_sfw();
    disconnect(this, SIGNAL(OTD_sfw()), mOTD, SLOT(OTD_fw()));
}

void SystemState::OTD_fw(double x)
{
    /*
    if (x == 2)
    {
        ui->error_mod->setText(" Не удалось узнать версию прошивки!");
    }
    else
    {
        ui->lineEdit_5->setText (QString::number(x/10));
    }
    */
}

void SystemState::OTDPTdata(double x,double y)
{
    x = x / 100;
    y = y / 100;
    //ui->OTDerror->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
    //if(x == -256) ui->OTDerror->setText("Ошибка измерения датчика");
    //if(y == -256) ui->OTDerror->setText("Ошибка измерения датчика");
    //if(x > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");
    //if(y > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");
    //ui->OTDPT1->setText(QString::number(x));
    //ui->OTDPT2->setText(QString::number(y));
}

void SystemState::on_OTDPT_clicked()
{
    connect(this, SIGNAL(OTD1()), mOTD, SLOT(OTDPT()));
    emit OTD1();
    disconnect(this, SIGNAL(OTD1()), mOTD, SLOT(OTDPT()));
}

void SystemState::OTDtemd(QString data)
{
    //ui->OTDtd->setText(data);
}

void SystemState::on_OTD_reset_1_clicked()
{
    connect(this, SIGNAL(OTD_reset1()), mOTD, SLOT(OTDres1()));
    emit OTD_reset1();
    disconnect(this, SIGNAL(OTD_reset1()), mOTD, SLOT(OTDres1()));
}

void SystemState::on_OTD_reset_2_clicked()
{
    connect(this, SIGNAL(OTD_reset2()), mOTD, SLOT(OTDres2()));
    emit OTD_reset2();
    disconnect(this, SIGNAL(OTD_reset2()), mOTD, SLOT(OTDres2()));
}

void SystemState::OTDerror(QString err)
{
   // ui->OTDerror->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
    //ui->OTDerror->setText(err);
}

void SystemState::on_OTD_meas_1_clicked()
{
    connect(this, SIGNAL(OTD_meas1()), mOTD, SLOT(OTDmeas1()));
    emit OTD_meas1();
    disconnect(this, SIGNAL(OTD_meas1()), mOTD, SLOT(OTDmeas1()));
}

void SystemState::on_OTD_meas_2_clicked()
{
    connect(this, SIGNAL(OTD_meas2()), mOTD, SLOT(OTDmeas2()));
    emit OTD_meas2();
    disconnect(this, SIGNAL(OTD_meas2()), mOTD, SLOT(OTDmeas2()));
}

void SystemState::OTDtm1(QString temp)
{
    //ui->OTDtm1->setText(temp);
}

void SystemState::OTDtm2(QString temp)
{
    //ui->OTDtm2->setText(temp);
}

void SystemState::on_OTD_nd_clicked()
{
    connect(this, SIGNAL(OTD_nd()), mOTD, SLOT(OTDtemper()));
    emit OTD_nd();
    disconnect(this, SIGNAL(OTD_nd()), mOTD, SLOT(OTDtemper()));
}

void SystemState::on_pow_DY_osn_clicked()
{
    /*
    QString S1 = ui->lineEdit_Addr_2->text();
    int u1 = S1.toInt();
    connect(this, SIGNAL(MKO_DY(int, int)), mMKO, SLOT(pow_DY(int, int)));
    emit MKO_DY(MKO::MAIN_KIT, u1);
    disconnect(this, SIGNAL(MKO_DY(int, int)), mMKO, SLOT(pow_DY(int, int)));
    */
}

void SystemState::on_pow_DY_rez_clicked()
{
    /*
    QString S2 = ui->lineEdit_Addr_3->text();
    int u2 = S2.toInt();
    connect(this, SIGNAL(MKO_DY(int, int)), mMKO, SLOT(pow_DY(int, int)));
    emit MKO_DY(MKO::RESERVE_KIT, u2);
    disconnect(this, SIGNAL(MKO_DY(int, int)), mMKO, SLOT(pow_DY(int, int)));
    */
}

void SystemState::on_MKO_osn_clicked()
{
    m_mko_kits ^= ModuleMKO::MAIN_KIT;

    if(m_mko_kits & ModuleMKO::MAIN_KIT)
    {
        //ui->MKO_osn->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else
    {
        //ui->MKO_osn->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }

    connect(this, SIGNAL(MKO_ch(int)), mMKO, SLOT(MKO_chan(int)));
    emit MKO_ch(m_mko_kits);
    disconnect(this, SIGNAL(MKO_ch(int)), mMKO, SLOT(MKO_chan(int)));
}

void SystemState::on_MKO_rez_clicked()
{
    m_mko_kits ^= ModuleMKO::RESERVE_KIT;
    if(m_mko_kits & ModuleMKO::RESERVE_KIT)
    {
        //ui->MKO_rez->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else
    {
        //ui->MKO_rez->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }

    connect(this, SIGNAL(MKO_ch(int)), mMKO, SLOT(MKO_chan(int)));
    emit MKO_ch(m_mko_kits);
    disconnect(this, SIGNAL(MKO_ch(int)), mMKO, SLOT(MKO_chan(int)));
}

void SystemState::on_MKO_test_clicked()
{
    /*
    QString S1 = ui->lineEdit_Addr_2->text();
    int u1 = S1.toInt();
    QString S2 = ui->lineEdit_Addr_3->text();
    int u2 = S2.toInt();
    connect(this, SIGNAL(MKO_ts(int,int,int)), mMKO, SLOT(MKO_start_test(int,int,int)));
    emit MKO_ts(m_mko_kits, u1, u2);
    disconnect(this, SIGNAL(MKO_ts(int,int,int)), mMKO, SLOT(MKO_start_test(int,int,int)));
    */
}

void SystemState::on_pushButton_11_clicked()
{
    //QString S1;
    //S1 += ui->MKO_cd_0->text()+" ";
    //S1 += ui->MKO_cd_1->text()+" ";
    //S1 += ui->MKO_cd_3->text()+" ";
    //S1 += ui->MKO_cd_4->text()+" ";
    //S1 += ui->MKO_cd_5->text()+" ";
    //S1 += ui->MKO_cd_6->text()+" ";
    //S1 += ui->MKO_cd_8->text()+" ";
    //S1 += ui->MKO_cd_9->text();
    //QString S3 = ui->lineEdit_Addr_2->text();
    //int u1 = S3.toInt();
    //QString S2 = ui->lineEdit_Addr_3->text();
    //int u2 = S2.toInt();
    //connect(this, SIGNAL(MKO_cm(int,QString, int,int)), mMKO, SLOT(MKO_tr_cm(int,QString,int,int)));
    //emit MKO_cm(m_mko_kits, S1, u1, u2);
    //disconnect(this, SIGNAL(MKO_cm(int,QString,int,int)), mMKO, SLOT(MKO_tr_cm(int,QString,int,int)));
}

void SystemState::on_pushButton_12_clicked()
{
    /*
    QString S1 = ui->lineEdit_Addr_2->text();
    int u1 = S1.toInt();
    QString S2 = ui->lineEdit_Addr_3->text();
    int u2 = S2.toInt();
    connect(this, SIGNAL(MKO_cm_r(int,int,int)), mMKO, SLOT(MKO_rc_cm(int,int,int)));
    emit MKO_cm_r(m_mko_kits, u1, u2);
    disconnect(this, SIGNAL(MKO_cm_r(int,int,int)), mMKO, SLOT(MKO_rc_cm(int,int,int)));
    */
}

void SystemState::MKO_change_ch(int x, int y)
{
    stm_on_mko(x,y);
}

void SystemState::on_MKO_avt_clicked()
{
    /*
    QString S1 = ui->lineEdit_Addr_2->text();
    int u1 = S1.toInt();
    QString S2 = ui->lineEdit_Addr_3->text();
    int u2 = S2.toInt();
    QString S3 = ui->lineEdit_period->text();
    int u3 = S3.toInt();
    if(flag_mko_auto == 0)
    {
        flag_mko_auto = 1;
        ui->MKO_avt->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
        connect(this, SIGNAL( MKO_auto(int,int,int,int)), mMKO, SLOT(MKO_avt(int,int,int,int)));
        emit MKO_auto(flag_mko_auto,u3,u1,u2);
        disconnect(this, SIGNAL( MKO_auto(int,int,int,int)), mMKO, SLOT(MKO_avt(int,int,int,int)));
    }
    else if(flag_mko_auto == 1)
    {
        flag_mko_auto = 0;
        ui->MKO_avt->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
        connect(this, SIGNAL( MKO_auto(int,int,int,int)), mMKO, SLOT(MKO_avt(int,int,int,int)));
        emit MKO_auto(flag_mko_auto,u3,u1,u2);
        disconnect(this, SIGNAL( MKO_auto(int,int,int,int)), mMKO, SLOT(MKO_avt(int,int,int,int)));
    }
    */
}

void SystemState::on_OTD_avt_2_clicked()
{
    /*
    QString S3 = ui->lineEdit_period_OTD->text();
    int u3 = S3.toInt();
    if(flag_otd_auto == 0)
    {
        flag_otd_auto = 1;
        ui->OTD_avt_2->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
        connect(this, SIGNAL(OTD_auto(int,int)), mOTD, SLOT(OTD_avt(int,int)));
        emit OTD_auto(flag_otd_auto, u3 * 1000);
        disconnect(this, SIGNAL(OTD_auto(int,int)), mOTD, SLOT(OTD_avt(int,int)));
    }
    else if(flag_otd_auto == 1)
    {
        flag_otd_auto = 0;
        ui->OTD_avt_2->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
        connect(this, SIGNAL(OTD_auto(int,int)), mOTD, SLOT(OTD_avt(int,int)));
        emit OTD_auto(flag_otd_auto,u3);
        disconnect(this, SIGNAL(OTD_auto(int,int)), mOTD, SLOT(OTD_avt(int,int)));
    }
    */
}

QSerialPort * SystemState::createPort(const QString& name)
{
    QSerialPort *port = new QSerialPort(name, this);
    port->open(QIODevice::ReadWrite);
    port->setBaudRate(QSerialPort::Baud115200);
    port->setDataBits(QSerialPort::Data5);
    port->setParity(QSerialPort::OddParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);

    return port;
}

QByteArray SystemState::send(QSerialPort * port, QByteArray data)
{
    QByteArray readData;
    if (port->isOpen())
    {
        port->QIODevice::write(data);
        port->waitForBytesWritten(-1);

        readData = port->readAll();
        while (port->waitForReadyRead(100))
        {
            readData.append(port->readAll());
        }
    }

    return readData;
}

void SystemState::startPower()
{
    // PowerON
    QByteArray buffer1(7, 0);
    buffer1[0] = 0xf1;
    buffer1[1] = 0x00;
    buffer1[2] = 0x36;
    buffer1[3] = 0x10;
    buffer1[4] = 0x10;
    buffer1[5] = 0x01;
    buffer1[6] = 0x47;
    QByteArray readData11 = send(getPort(POW_ANT_DRV_CTRL), buffer1);
    QByteArray readData21 = send(getPort(POW_ANT_DRV), buffer1);

    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;//power off
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x01;
    buffer[4] = 0x00;
    buffer[5] = 0x01;
    buffer[6] = 0x28;

    QByteArray readData1 = send(getPort(POW_ANT_DRV_CTRL), buffer);
    QByteArray readData2 = send(getPort(POW_ANT_DRV), buffer);

    setVoltageAndCurrent(POW_ANT_DRV, 0.5);
    setVoltageAndCurrent(POW_ANT_DRV_CTRL, 0.5);
}

int SystemState::setPowerChannelState(int channel, PowerState state)
{
    QByteArray buffer(4, 0);
    buffer[0] = STM_DEFAULT_ADDR;
    buffer[1] = ModuleCommands::POWER_CHANNEL_CTRL;
    buffer[2] = channel;
    buffer[3] = (state == POWER_ON) ? 1 : 0;

    QByteArray readData1 = send(getPort(STM), buffer);
    return readData1[3];
}

int SystemState::stm_on_mko(int x, int y)
{
    QByteArray buffer(4, 0);
    buffer[0] = STM_DEFAULT_ADDR;
    buffer[1] = ModuleCommands::SET_MKO_PWR_CHANNEL_STATE;
    buffer[2] = x;
    buffer[3] = y;

    QByteArray readData1 = send(getPort(STM), buffer);
    return readData1[3];
}

int SystemState::stm_check_fuse(int fuse)
{
    QByteArray buffer(4, 0);
    buffer[0] = STM_DEFAULT_ADDR;
    buffer[1] = ModuleCommands::GET_PWR_MODULE_FUSE_STATE;
    buffer[2] = fuse;
    buffer[3] = 0x00;
    QByteArray readData1 = send(getPort(STM), buffer);
    return readData1[3];
}

int SystemState::tech_send(int com, int x, int y)
{
    QByteArray buffer(4, 0);
    buffer[0] = TECH_DEFAULT_ADDR;
    buffer[1] = com;
    buffer[2] = x;
    buffer[3] = y;
    QByteArray readData1 = send(getPort(TECH), buffer);
    return readData1[3];
}

int SystemState::tech_read(int x)
{
    if(isActive(TECH))
    {
        uint8_t command = 0;
        if(x == 1)
        {
            command = ModuleCommands::CHECK_RECV_DATA_RS485;
        }
        else
        {
            command = ModuleCommands::CHECK_RECV_DATA_CAN;
        }

        QByteArray buffer(4, 0);
        buffer[0] = TECH_DEFAULT_ADDR;
        buffer[1] = command;
        buffer[2] = 0x00;
        buffer[3] = 0x00;
        QByteArray readData2 = send(getPort(TECH), buffer);

        uint8_t uu1,uu2;
        uu1 = readData2[2];
        uu2 = readData2[3];
        double uu = (uu1 << 8) | uu2;
        return uu;
    }

    return 0;
}

QString SystemState::tech_read_buf(int x, int len)
{
    uint8_t command = 0;
    if(x == 1)
    {
        command = ModuleCommands::RECV_DATA_RS485;
    }
    else
    {
        command = ModuleCommands::RECV_DATA_CAN;
    }

    QString result;
    QByteArray buffer(4, 0);
    for(int i = 0; i < len; i++)
    {
        buffer[0] = TECH_DEFAULT_ADDR;
        buffer[1] = command;
        buffer[2] = 0x00;
        buffer[3] = 0x00;
        QByteArray readData2 = send(getPort(TECH), buffer);

        if(readData2.at(2) == 1)
        {
            result += "em ";
        }

        if(readData2.at(2) == 2)
        {
            result += "uu ";
        }

        result += readData2[3];
        result += " ";
        QApplication::processEvents();
    }

    return result;
}

double SystemState::stm_data_ch(int ch)
{
    if (isActive(STM))
    {
        QByteArray buffer(4, 0);
        buffer[0] = STM_DEFAULT_ADDR;
        buffer[1] = ModuleCommands::GET_CHANNEL_TELEMETRY;
        buffer[2] = ch;
        buffer[3] = 0x00;

        QByteArray readData1 = send(getPort(STM), buffer);

        uint8_t uu1, uu2;
        uu1 = readData1[2];
        uu2 = readData1[3];
        double res = (uu1 << 8) | uu2;
        return res;
    }

    return 50000;
}

QSerialPort* SystemState::getPort(ModuleID id)
{
    QSerialPort* port = m_modules.value(id, ModuleInfo()).port;
    Q_ASSERT(port != Q_NULLPTR);
    return port;
}

int SystemState::resetError(ModuleID id)
{
    QSerialPort * port = getPort(id);
    QByteArray buffer;

    switch (id)
    {
    case POW_ANT_DRV_CTRL:
    case POW_ANT_DRV:
        {
            buffer.resize(7);
            buffer[0] = 0xf1;
            buffer[1] = 0x00;
            buffer[2] = 0x36;
            buffer[3] = 0x0a;
            buffer[4] = 0x0a;
            buffer[5] = 0x01;
            buffer[6] = 0x3b;
        }
        break;

    case STM:
        {
            buffer.resize(4);
            buffer[0] = STM_DEFAULT_ADDR;
            buffer[1] = ModuleCommands::RESET_ERROR;
            buffer[2] = 0x00;
            buffer[3] = 0x00;
        }
        break;

    case TECH:
        {
            buffer.resize(4);
            buffer[0] = TECH_DEFAULT_ADDR;
            buffer[1] = ModuleCommands::RESET_ERROR;
            buffer[2] = 0x00;
            buffer[3] = 0x00;
        }
        break;
    default:
        break;
    }

    QByteArray readData = send(port, buffer);
    if (readData.size() > 3)
    {
        return readData[3];
    }

    return 0;
}

void SystemState::setPowerState(ModuleID id, PowerState state)
{
    QSerialPort * port = getPort(id);
    Q_ASSERT(id == POW_ANT_DRV_CTRL || id == POW_ANT_DRV);

    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;//power on/off
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x01;
    buffer[4] = 0x01;
    buffer[5] = 0x01;
    buffer[6] = (state == POWER_ON) ? 0x29 : 0x28;
    QByteArray readData1 = send(port, buffer);
}

void SystemState::setPowerValue(uint8_t valueID, double value, double maxValue, QSerialPort * port)
{
    QByteArray buffer(7, 0);
    uint32_t val = uint32_t((value * 256 * 100) / maxValue);

    if(val > (256 * 100))
    {
        val = (256 * 100);
    }

    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = valueID;
    buffer[3] = (val >> 8) & 0xFF;
    buffer[4] = val & 0xFF;
    uint16_t sum = 0;
    for(int i = 0; i < 5; i++)
    {
        uint8_t s = buffer[i];
        sum = (sum + s) & 0xFFFF;
    }

    buffer[5] = ((sum >> 8) & 0xFF);
    buffer[6] = (sum & 0xFF);
    QByteArray readData = send(port, buffer);
}

void SystemState::setMaxVoltageAndCurrent(ModuleID id, double voltage, double current)
{
    QSerialPort * port = getPort(id);
    Q_ASSERT(id == POW_ANT_DRV_CTRL || id == POW_ANT_DRV);

    // TODO make constants
    setPowerValue(MAX_VOLTAGE_VAL, voltage, MAX_VOLTAGE, port);
    setPowerValue(MAX_CURRENT_VAL, current, MAX_CURRENT, port);
}

void SystemState::setVoltageAndCurrent(ModuleID id, double voltage)
{
    QSerialPort * port = getPort(id);
    Q_ASSERT(id == POW_ANT_DRV_CTRL || id == POW_ANT_DRV);

    // TODO make constants
    setPowerValue(CUR_VOLTAGE_VAL, voltage, MAX_VOLTAGE, port);
    setPowerValue(CUR_CURRENT_VAL, ((double)MAX_POWER) / voltage, MAX_CURRENT, port);
}

void SystemState::getCurVoltageAndCurrent(ModuleID id, double& voltage, double& current, uint8_t& error)
{
    QSerialPort * port = getPort(id);
    Q_ASSERT(id == POW_ANT_DRV_CTRL || id == POW_ANT_DRV);

    QByteArray buffer(5, 0);
    buffer[0] = 0x75;
    buffer[1] = 0x00;
    buffer[2] = 0x47;
    buffer[3] = 0x00;
    buffer[4] = 0xbc;

    QByteArray readData = send(port, buffer);

    uint8_t uu1, uu2;
    error = (readData[4] >> 4);

    uu1 = readData[5];
    uu2 = readData[6];
    voltage = (uu1 << 8) | uu2;
    voltage = voltage * MAX_VOLTAGE / 256;

    uu1 = readData[7];
    uu2 = readData[8];
    current = (uu1 << 8) | uu2;
    current = current * MAX_CURRENT / 256;
}

int SystemState::id_stm()
{
    QByteArray buffer(4, 0);
    buffer[0] = 0xff;
    buffer[1] = ModuleCommands::GET_MODULE_ADDRESS;
    buffer[2] = 0x00;
    buffer[3] = ModuleCommands::CURRENT;
    QByteArray readData1 = send(getPort(STM), buffer);

    buffer[0] = 0xff;
    buffer[1] = ModuleCommands::GET_MODULE_ADDRESS;
    buffer[2] = 0x00;
    buffer[3] = ModuleCommands::DEFAULT;
    QByteArray readData2 = send(getPort(STM), buffer);

    if(readData1[2] == readData2[2] && readData1[3] == readData2[3])
    {
        return 1;
    }

    return 0;
}

int SystemState::id_tech()
{
    QByteArray buffer(4, 0);
    buffer[0] = 0xff;
    buffer[1] = ModuleCommands::GET_MODULE_ADDRESS;
    buffer[2] = 0x00;
    buffer[3] = ModuleCommands::CURRENT;
    QByteArray readData1 = send(getPort(TECH), buffer);

    buffer[0] = 0xff;
    buffer[1] = ModuleCommands::GET_MODULE_ADDRESS;
    buffer[2] = 0x00;
    buffer[3] = ModuleCommands::DEFAULT;

    QByteArray readData2 = send(getPort(TECH), buffer);

    if (readData1[2] == readData2[2] && readData1[3] == readData2[3])
    {
        return 1;
    }

    return 0;
}

QString SystemState::req_stm()
{
    if (isActive(STM))
    {
        QString res;
        QByteArray buffer(4, 0);
        buffer[0] = STM_DEFAULT_ADDR;
        buffer[1] = ModuleCommands::GET_STATUS_WORD;
        buffer[2] = 0x00;
        buffer[3] = 0x00;
        QByteArray readData1 = send(getPort(STM), buffer);
        uint8_t x = readData1[2];
        uint8_t z1, z2, z3;
        z1 = x >> 7;
        z2 = x << 1;
        z2 = z2 >> 7;
        z3 = x << 2;
        z3 = z3 >> 7;
        if(z1 == 0)
            res += " СТМ не готов к работе! \n";
        if(z2 == 1)
            res += " Ошибки у модуля СТМ! \n";
        if(z3 == 1)
            res += " Модуль СТМ после перезагрузки! \n";
        return res;
    }

    return "";
}

QString SystemState::req_tech()
{
    if(isActive(TECH))
    {
        QString res;
        QByteArray buffer(4, 0);
        buffer[0] = TECH_DEFAULT_ADDR;
        buffer[1] = ModuleCommands::GET_STATUS_WORD;
        buffer[2] = 0x00;
        buffer[3] = 0x00;

        QByteArray readData1 = send(getPort(TECH), buffer);
        uint8_t x = readData1[2];
        uint8_t z1, z2, z3;
        z1 = x >> 7;
        z2 = x << 1;
        z2 = z2 >> 7;
        z3 = x << 2;
        z3 = z3 >> 7;
        if(z1 == 0)
            res += " Технол. модуль не готов к работе! \n";
        if(z2 == 1)
            res += " Ошибки у Технол. модуля! \n";
        if(z3 == 1)
            res += " Модуль Технол. после перезагрузки! \n";
        if(readData1.at(3)==0x10)
            res += " Потеря байта из-за переполнения буфера RS485! \n";
        return res;
    }

    return "";
}

int SystemState::softResetModule(ModuleID id)
{
    Q_ASSERT(id == STM || id == TECH);

    setActive(id, false);

    uint8_t moduleAddr = STM_DEFAULT_ADDR;
    if (id == TECH)
    {
        moduleAddr = TECH_DEFAULT_ADDR;
    }

    QByteArray buffer(4, 0);
    buffer[0] = moduleAddr;
    buffer[1] = ModuleCommands::SOFT_RESET;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    QSerialPort * port = getPort(id);
    QByteArray readData1 = send(port, buffer);

    resetPort(id);
    setActive(id, true);

    return readData1[3];
}

void SystemState::resetPort(ModuleID id)
{
    ModuleInfo info = m_modules.take(id);

    info.port->close();
    info.port->deleteLater();
    info.port = Q_NULLPTR;

    for(int i = 0; i < 500; i++) // i guess some sort of govnomagics here "freeze app for 5 seconds to restore COM port with module after reset"
    {
        Sleep(10);
        QApplication::processEvents();
    }

    info.port = createPort("");
    m_modules[id] = info;
}

bool SystemState::isActive(ModuleID id) const
{
    return m_modules.value(id, ModuleInfo()).state;
}

void SystemState::setActive(ModuleID id, bool state)
{
    ModuleInfo info = m_modules.take(id);
    info.state = state;
    m_modules[id] = info;
}

int SystemState::getSoftwareVersion(ModuleID id)
{
    QSerialPort * port = getPort(id);
    Q_ASSERT(id == STM || id == TECH);

    uint8_t moduleAddr = STM_DEFAULT_ADDR;
    if (id == TECH)
    {
        moduleAddr = TECH_DEFAULT_ADDR;
    }

    QByteArray buffer(4, 0);
    buffer[0] = moduleAddr;
    buffer[1] = ModuleCommands::GET_SOWFTWARE_VER;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    QByteArray readData1 = send(port, buffer);
    return (readData1[2] * 10 + readData1[3]); // версия прошивки, ИМХО неправильно считается, т.к. два байта на нее
}
