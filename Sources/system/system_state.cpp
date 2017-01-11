#include "Headers/system/system_state.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/modules/module_otd.h"
#include "Headers/system/modules/module_mko.h"
#include "Headers/system/modules/module_power.h"
#include "Headers/system/modules/module_stm.h"
#include "Headers/system/modules/module_tech.h"

#include <QtSerialPort>
#include <windows.h>
#include "qapplication.h"


namespace
{
    static const uint8_t OTD_DEFAULT_ADDR = 0x44;
    static const uint8_t STM_DEFAULT_ADDR = 0x22;
    static const uint8_t TECH_DEFAULT_ADDR = 0x56;
}

SystemState::SystemState(QObject* parent):
    VariableController(parent),
    m_mko_kits(ModuleMKO::NO_KIT),
    mThreadMKO(Q_NULLPTR),
    mThreadOTD(Q_NULLPTR),
    PAR_VOLTAGE(tr("Напряжение, В")),
    PAR_CURRENT(tr("Ток, А")),
    mCurCommand(Q_NULLPTR)
{
}

SystemState::~SystemState()
{
    if (mThreadMKO && mThreadMKO->isRunning())
    {
        mThreadMKO->terminate();
    }

    if (mThreadOTD && mThreadOTD->isRunning())
    {
        mThreadOTD->terminate();
    }

    if (mMKO)
    {
        delete mMKO;
    }

    if (mOTD)
    {
        delete mOTD;
    }
}

void SystemState::init()
{
    //return;

    /*
     * Что надо сделать?
     *
     * 1. Тупо создаем все модули в НУЖНОМ ПОРЯДКЕ!
     * 2. Создаем два модуля питания, оставляя один класс module_power, избавляясь от module_power_bup и module_power_pna
     * 3. класс Module переиименовываем в COMPortModule и переносим туда весь функционал работы с COM-портом
     * 4. туда же наверное переносим команды, принимаемые всеми "нашими" модулями, но делаем их виртуальными и в модуле питания их переопределяем на пустышки
     * 5. здесь создаем объекты всех модулей в нужном порядке
     * 6. здесь же из инитим, безо всякой поебени типа "стартовали тред-шлем сигнал себе-инициализируемся"
     * 7. пусть инит пройдет здесь, а далее каждый тред живет в своем потоке
     * 8. свой поток важен в том плане, что какой-то модуль может затупить, и, если это случится, то повиснет все, а не только поток модуля
     * 9. модули с систем стэйтом будут общаться через сигналы-слоты
     * 10. систем стэйт по идее будет просто слушать модули и слать сигналв в их слоты
     * 11. напрямую модули нихера не выдают
     * 12. также систем стэйт держит флажки активности модулей, которые при перезагрузках как-то модифицируются
     * 13. MyClass вообще удалить нахуй
     *
     * Далее, общий принцип работы такой:
     *
     * 1. Модули вертятся каждый в своем потоке
     * 2. Они обновляются по таймеру и серут сигналами, что тот или иной параметр обновился
     * 3. System State аккумулирует то, что модули серут и через сигналы их может попинывать
     * 4. Внутри себя модули не хранят данные
     *
     *
    */

    //QThread* mThreadOTD;
    //QThread* mThreadMKO;
    //QThread* mThreadSTM;
    //QThread* mThreadTech;
    //QThread* mThreadPowerBUP;
    //QThread* mThreadPowerPNA;

    setupParams();

    // TODO: The order of modules creation possibly important!
    mPowerPNA = new ModulePower(this);
    mPowerBUP = new ModulePower(this);
    mSTM = new ModuleSTM(this);
    mTech = new ModuleTech(this);

    mOTD = new ModuleOTD(Q_NULLPTR);
    //connect(mOTD, SIGNAL(start_OTDPT(double,double)), this, SLOT(OTDPTdata(double,double)));
    //connect(mOTD, SIGNAL(temp_OTD(QString)), this, SLOT(OTDtemd(QString)));
    //connect(mOTD, SIGNAL(OTD_res(int)), this, SLOT(OTD_res_st(int)));
    //connect(mOTD, SIGNAL(OTD_reqr(QString)), this, SLOT(status_OTD(QString)));
    //connect(mOTD, SIGNAL(OTD_err_res(int)), this, SLOT(OTD_err_res(int)));
    //connect(mOTD, SIGNAL(OTD_id1()), this, SLOT(OTD_id()));
    //connect(mOTD, SIGNAL(OTD_vfw(double)), this, SLOT(OTD_fw(double)));
    //connect(mOTD, SIGNAL(err_OTD(QString)), this, SLOT(OTDerror(QString)));
    //connect(mOTD, SIGNAL(tm_OTD1(QString)), this, SLOT(OTDtm1(QString)));
    //connect(mOTD, SIGNAL(tm_OTD2(QString)), this, SLOT(OTDtm2(QString)));

    mMKO = new ModuleMKO(Q_NULLPTR);
    //connect(mMKO, SIGNAL(test_MKO(int)), this, SLOT(simpltst1(int)));
    //connect(mMKO, SIGNAL(MKO_CTM(int, int)), this, SLOT(MKO_change_ch(int, int)));
    //connect(mMKO, SIGNAL(start_MKO(QString)), this, SLOT(MKO_data(QString)));
    //connect(mMKO, SIGNAL(data_MKO(QString)), this, SLOT(MKO_cm_data(QString)));

    mPowerPNA->init();
    mPowerBUP->init();
    mSTM->init();
    mTech->init();
    mOTD->init();

    mPowerBUP->startPower();
    //mPowerBUP->setUpdatePeriod(5000); //TODO

    mPowerPNA->startPower();
    //mPowerPNA->setUpdatePeriod(5000); //TODO

    mSTM->stm_on_mko(1, 0);
    mSTM->stm_on_mko(2, 0);

    mThreadMKO = new QThread(this);
    mMKO->moveToThread(mThreadMKO);
    mThreadMKO->start();

    mThreadOTD = new QThread(this);
    mOTD->moveToThread(mThreadOTD);
    connect(mThreadOTD, SIGNAL(started()), mOTD, SLOT(COMConnectorOTD()));
    mThreadOTD->start();

    //QThread *thread = new QThread;
    //MyClass *my = new MyClass("B");
    //my->moveToThread(thread);
    //connect(my, SIGNAL(send()), this, SLOT(paintvalue()));
    //connect(my, SIGNAL(send3()), this, SLOT(statusRS()));
    //connect(my, SIGNAL(send4()), this, SLOT(statusCAN()));
    //connect(my, SIGNAL(send5()), this, SLOT(checkModulesStatus()));
    //connect(thread, SIGNAL(started()), my, SLOT(doWork()));
    //thread->start();

    //startpower(); // Ivan Semenchuk: what for to do it twice?
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

//void SystemState::paintvalue()
//{
//    double u1, u2, i1, i2;
//    uint8_t er1, er2;

    //ui->tech_error->setText("");
    //getCurVoltageAndCurrent(POW_ANT_DRV, u1, i1, er1);
    //getCurVoltageAndCurrent(POW_ANT_DRV_CTRL, u2, i2, er2);

    //ui->err1->setStyleSheet("font: 25 9pt GOST type A;" "color: black;");
    //ui->err2->setStyleSheet("font: 25 9pt GOST type A;" "color: black;");
    //if(er1 != 0)
    //{
        //m_flag_rem1 = 0;
        //ui->pushButton_start_com6->setChecked(false);
    //}

    //if(er2 != 0)
    //{
        //m_flag_rem2 = 0;
        //ui->pushButton_start_com5->setChecked(false);
    //}

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

    //if(m_k > 500) // wtf?
    //{
    //    m_k = 0;
    //}

    //m_dat[m_k] = i1 / 100;
    //m_dat1[m_k] = i2 / 100;
    //m_k++;
    //plot_point();
//}

void SystemState::checkModulesStatus()
{
    //TODO здесь тупро проверка статуса по времени, не отвалилось ли чего
    //QString res;
    //on_pushButton_ctm_ch_15_clicked(); // состояние какого-то служебного канала
    //res += req_stm();
    //res += req_tech();
    //connect(this, SIGNAL(OTD_req()), mOTD, SLOT(OTD_req()));
    //emit OTD_req();
    //disconnect(this, SIGNAL(OTD_req()), mOTD, SLOT(OTD_req()));
    //ui->error_mod->setText(res);
}

void SystemState::status_OTD(QString data)
{
    if(data != "")
    {
        //ui->error_mod->setText (data);
    }
}

/*void SystemState::plot_point()
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
*/

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

void SystemState::OTDtemd(QString data)
{
    //ui->OTDtd->setText(data);
}

void SystemState::OTDerror(QString err)
{
   // ui->OTDerror->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
    //ui->OTDerror->setText(err);
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
    mSTM->stm_on_mko(x,y);
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

ModuleMKO* SystemState::moduleMKO() const
{
    return mMKO;
}

ModuleOTD* SystemState::moduleOTD() const
{
    return mOTD;
}

ModuleSTM* SystemState::moduleSTM() const
{
    return mSTM;
}

ModuleTech* SystemState::moduleTech() const
{
    return mTech;
}

ModulePower* SystemState::modulePowerBUP() const
{
    return mPowerBUP;
}

ModulePower* SystemState::modulePowerPNA() const
{
    return mPowerPNA;
}

QString SystemState::paramName(int module, int command, int param, bool isInputParam) const
{
    if (module >= 0 && module < ModuleCommands::MODULES_COUNT)
    {
        const QMap<int, QStringList>& container = (isInputParam ? mInParams[module] : mOutParams[module]);
        QMap<int, QStringList>::const_iterator it = container.find(command);
        if (it != container.end())
        {
            if (param >= 0 && param < it.value().size())
            {
                return it.value().at(param);
            }
        }
    }

    return "";
}

int SystemState::paramsCount(int module, int command, bool isInputParam) const
{
    if (module >= 0 && module < ModuleCommands::MODULES_COUNT)
    {
        const QMap<int, QStringList>& container = isInputParam ? mInParams[module] : mOutParams[module];
        QMap<int, QStringList>::const_iterator it = container.find(command);
        if (it != container.end())
        {
            return it.value().size();
        }
    }

    return 0;
}

void SystemState::setupParams()
{
    QStringList powerParams({PAR_VOLTAGE, PAR_CURRENT});
    {
        QMap<int, QStringList> params;
        params[ModuleCommands::SET_VOLTAGE_AND_CURRENT] = powerParams;
        params[ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT] = powerParams;
        mInParams[ModuleCommands::POWER_UNIT_BUP] = params;

        params.clear();
        params[ModuleCommands::GET_VOLTAGE_AND_CURRENT] = powerParams;
        mOutParams[ModuleCommands::POWER_UNIT_BUP] = params;
    }

    {
        QMap<int, QStringList> params;
        params[ModuleCommands::SET_VOLTAGE_AND_CURRENT] = powerParams;
        params[ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT] = powerParams;
        mInParams[ModuleCommands::POWER_UNIT_PNA] = params;

        params[ModuleCommands::GET_VOLTAGE_AND_CURRENT] = powerParams;
        mOutParams[ModuleCommands::POWER_UNIT_PNA] = params;
    }

    {
        QMap<int, QStringList> params;
        mInParams[ModuleCommands::MKO] = params;
    }

    {
        QMap<int, QStringList> params;
        mInParams[ModuleCommands::STM] = params;
    }

    {
        QMap<int, QStringList> params;
        mInParams[ModuleCommands::OTD] = params;
    }

    {
        QMap<int, QStringList> params;
        mInParams[ModuleCommands::TECH] = params;
    }

    /*
    GET_MODULE_ADDRESS              = 0x01,
    GET_STATUS_WORD                 = 0x02,
    RESET_ERROR                     = 0x03,
    SOFT_RESET                      = 0x04,
    //RESERVED_0x05 = 0x05,
    GET_SOWFTWARE_VER               = 0x06,
    ECHO                            = 0x07,
    //RESERVED_0x08 = 0x08,
    //RESERVED_0x09 = 0x09,
    //RESERVED_0x0A = 0x0A,
    POWER_CHANNEL_CTRL              = 0x0B, // Can be sent to STM only
    GET_PWR_MODULE_FUSE_STATE       = 0x0C, // Can be sent to STM only
    GET_CHANNEL_TELEMETRY           = 0x0D, // Can be sent to STM only
    SET_MKO_PWR_CHANNEL_STATE       = 0x0E, // Can be sent to STM only
    //MATRIX_CMD_CTRL                 = 0x0F, // Can be sent to MKU only
    SET_PACKET_SIZE_CAN             = 0x10, // Can be sent to TECH only
    ADD_BYTES_CAN                   = 0x11, // Can be sent to TECH only
    SEND_PACKET_CAN                 = 0x12, // Can be sent to TECH only
    CHECK_RECV_DATA_CAN             = 0x13, // Can be sent to TECH only
    RECV_DATA_CAN                   = 0x14, // Can be sent to TECH only
    CLEAN_BUFFER_CAN                = 0x15, // Can be sent to TECH only
    SET_PACKET_SIZE_RS485           = 0x16, // Can be sent to TECH only
    ADD_BYTES_RS485                 = 0x17, // Can be sent to TECH only
    SEND_PACKET_RS485               = 0x18, // Can be sent to TECH only
    CHECK_RECV_DATA_RS485           = 0x19, // Can be sent to TECH only
    RECV_DATA_RS485                 = 0x1A, // Can be sent to TECH only
    CLEAN_BUFFER_RS485              = 0x1B, // Can be sent to TECH only
    GET_TEMPERATURE_PT100           = 0x1C, // Can be sent to OTD only
    GET_DS1820_COUNT_LINE_1         = 0x1D, // Can be sent to OTD only (Psi)
    GET_DS1820_COUNT_LINE_2         = 0x1E, // Can be sent to OTD only (Nu)
    GET_TEMPERATURE_DS1820_LINE_1   = 0x1F, // Can be sent to OTD only (Psi)
    GET_TEMPERATURE_DS1820_LINE_2   = 0x20, // Can be sent to OTD only (Nu)
    GET_POWER_MODULE_STATE          = 0x21, // Can be sent to STM only
    //GET_MKU_MODULE_STATE            = 0x22, // Can be sent to MKU only
    GET_MKO_MODULE_STATE            = 0x23, // Can be sent to STM only
    SET_MODE_RS485                  = 0x24, // Can be sent to TECH only
    SET_SPEED_RS485                 = 0x25, // Can be sent to TECH only
    RESET_LINE_1                    = 0x26, // Can be sent to OTD only (Psi)
    RESET_LINE_2                    = 0x27, // Can be sent to OTD only (Nu)
    START_MEASUREMENT_LINE_1        = 0x28, // Can be sent to OTD only (Psi) 1-2 seconds to perform
    START_MEASUREMENT_LINE_2        = 0x29, // Can be sent to OTD only (Nu) 1-2 seconds to perform
    GET_DS1820_ADDR_LINE_1          = 0x2A, // Can be sent to OTD only (Psi)
    GET_DS1820_ADDR_LINE_2          = 0x2B, // Can be sent to OTD only (Nu)
    SET_TECH_INTERFACE              = 0x3B, // Can be sent to TECH only
    GET_TECH_INTERFACE              = 0x3C, // Can be sent to TECH only
    RECV_DATA_SSI                   = 0x3D, // Can be sent to TECH only

    // Third party modules commands (arbitrary) >>>>

    // Power unit modules commands (0xFF00-started)
    SET_VOLTAGE_AND_CURRENT         = 0xFF01,
    SET_MAX_VOLTAGE_AND_CURRENT     = 0xFF02,
    SET_POWER_STATE                 = 0xFF03,
    GET_CURRENT_VOLTAGE_AND_CURRENT = 0xFF04,
    */
}

void SystemState::sendCommand(CmdActionModule* command)
{
    bool result = false;
    // 1. По типу модуля взять модуль из имеющихся
    // 2. По типу команды определить сигнал, к которму коннектиться
    // 3. Послать модулю сигнал на выполнение той или иной команды
    // 4. Когда модуль получит сигнал и отпроцует команду, он пошлет сигнал, на который мы приконнектились
    // 5. В слоте мы полученными значениями проинициализируем переменные VariableController'а (если это выходные параметры)
    // 6. Посылаем сигнал commandFinished()
    // 7. Disconnect'имся от команды

    switch (command->module())
    {
    case ModuleCommands::POWER_UNIT_BUP:
    case ModuleCommands::POWER_UNIT_PNA:
        result = sendPowerUnitCommand(command);
        break;
    case ModuleCommands::OTD:
        result = sendOTDCommand(command);
        break;
    case ModuleCommands::STM:
        result = sendSTMCommand(command);
        break;
    case ModuleCommands::MKO:
        result = sendMKOCommand(command);
        break;
    case ModuleCommands::TECH:
        result = sendTechCommand(command);
        break;

    default:
        break;
    }

    if (!result)
    {
        emit commandFinished(result);
    }
}

bool SystemState::sendPowerUnitCommand(CmdActionModule* command)
{
    bool result = true;

    mCurCommand = command;

    ModulePower* module = (command->module() == ModuleCommands::POWER_UNIT_BUP ? mPowerBUP : mPowerPNA);
    const QMap<QString, QString>& inputParams = command->inputParams();
    //const QMap<QString, QString>& outputParams = command->outputParams();
    VariableController* varCtrl = command->variableController();

    switch (command->operation())
    {
    case ModuleCommands::GET_VOLTAGE_AND_CURRENT:
        {
            connect(this, SIGNAL(getUI()), module, SLOT(voltageAndCurrent()));
            connect(module, SIGNAL(gotUI(qreal,qreal,uint8_t)), this, SLOT(onUIGot(qreal, qreal, uint8_t)));
            emit getUI();
        }
        break;

    case ModuleCommands::SET_VOLTAGE_AND_CURRENT:
        {
            QString vName = inputParams.value(PAR_VOLTAGE, "ERR");
            QString cName = inputParams.value(PAR_CURRENT, "ERR");

            qreal voltage = varCtrl->variable(vName);
            qreal current = varCtrl->variable(cName);

            connect(this, SIGNAL(setUI(qreal,qreal)), module, SLOT(setVoltageAndCurrent(qreal, qreal)));
            connect(module, SIGNAL(gotUI(QString, qreal, QString, qreal,uint8_t)), this, SLOT(onUIChanged(qreal, qreal)));
            emit setUI(voltage, current);
        }
        break;

    case ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT:
        {
            QString vName = inputParams.value(PAR_VOLTAGE, "ERR");
            QString cName = inputParams.value(PAR_CURRENT, "ERR");

            qreal voltage = varCtrl->variable(vName);
            qreal current = varCtrl->variable(cName);

            connect(this, SIGNAL(setUI(qreal,qreal)), module, SLOT(setMaxVoltageAndCurrent(qreal, qreal)));
            connect(module, SIGNAL(changedUI(qreal,qreal)), this, SLOT(onUIChanged(qreal, qreal)));
            emit setUI(voltage, current);
        }
        break;

    case ModuleCommands::SET_POWER_STATE:
        {

        }
        break;

    default:
        result = false;
        break;
    }

    return result;
}

void SystemState::onUIChanged(qreal voltage, qreal current)
{
    QObject* sender = QObject::sender();
    if (sender)
    {
        disconnect(this, SIGNAL(setUI(qreal,qreal)), sender, SLOT(setMaxVoltageAndCurrent(qreal, qreal)));
        disconnect(this, SIGNAL(setUI(qreal,qreal)), sender, SLOT(setVoltageAndCurrent(qreal, qreal)));
        disconnect(sender, SIGNAL(changedUI(qreal,qreal)), this, SLOT(onUIChanged(qreal, qreal)));
    }

    onExecutionFinished(0);
}

void SystemState::onUIGot(qreal voltage, qreal current, uint8_t error)
{
    QObject* sender = QObject::sender();
    if (sender)
    {
        disconnect(this, SIGNAL(getUI()), sender, SLOT(voltageAndCurrent()));
        disconnect(sender, SIGNAL(gotUI(qreal,qreal,uint8_t)), this, SLOT(onUIGot(qreal, qreal, uint8_t)));
    }

    const QMap<QString, QString>& outputParams = mCurCommand->outputParams();
    VariableController* varCtrl = mCurCommand->variableController();
    QString vName = outputParams.value(PAR_VOLTAGE, "ERR");
    QString cName = outputParams.value(PAR_CURRENT, "ERR");

    varCtrl->setVariable(vName, voltage);
    varCtrl->setVariable(cName, current);

    onExecutionFinished(error);
}

void SystemState::onExecutionFinished(uint8_t error)
{
    int TODO; // process error

    mCurCommand = Q_NULLPTR;

    emit commandFinished(true);
}

bool SystemState::sendOTDCommand(CmdActionModule* command)
{
    int TODO;
    return false;
}

bool SystemState::sendSTMCommand(CmdActionModule* command)
{
    int TODO;
    return false;
}

bool SystemState::sendMKOCommand(CmdActionModule* command)
{
    int TODO;
    return false;
}

bool SystemState::sendTechCommand(CmdActionModule* command)
{
    int TODO;
    return false;
}
