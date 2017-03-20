#include "Headers/system/system_state.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/modules/module_otd.h"
#include "Headers/system/modules/module_mko.h"
#include "Headers/system/modules/module_power.h"
#include "Headers/system/modules/module_stm.h"
#include "Headers/system/modules/module_tech.h"

#include "Headers/logger/Logger.h"

#include <QMetaEnum>
#include <QtSerialPort>
#include <windows.h>
#include "qapplication.h"

namespace
{
    bool loadSystemConfig(QMap<ModuleCommands::ModuleID, COMPortModule::Identifier>& modules)
    {
        modules.clear();

        QXmlStreamReader xml;

        QString fileName = QDir::currentPath() + "/system_config.xml";
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            LOG_FATAL("No system_config.xml found");
            //QMessageBox::warning(this, tr("OKB5 Test Studio"), tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
            return false;
        }

        xml.setDevice(&file);
        QMetaEnum metaEnum = QMetaEnum::fromType<ModuleCommands::ModuleID>();

        if (xml.readNextStartElement())
        {
            if (xml.name() == "system_config" && xml.attributes().value("version") == "1.0")
            {
                while (!xml.atEnd() && !xml.hasError())
                {
                    QXmlStreamReader::TokenType token = xml.readNext();

                    if (token == QXmlStreamReader::StartElement)
                    {
                        QString name = xml.name().toString();

                        if (name == "modules")
                        {
                            while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "modules"))
                            {
                                if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "module")
                                {
                                    QXmlStreamAttributes attributes = xml.attributes();

                                    COMPortModule::Identifier id;
                                    ModuleCommands::ModuleID moduleID = ModuleCommands::MODULES_COUNT;

                                    if (attributes.hasAttribute("type"))
                                    {
                                        QString str = attributes.value("type").toString();
                                        moduleID = ModuleCommands::ModuleID(metaEnum.keyToValue(qPrintable(str)));
                                    }

                                    if (attributes.hasAttribute("description"))
                                    {
                                        id.description = attributes.value("description").toString();
                                    }

                                    if (attributes.hasAttribute("productId"))
                                    {
                                        id.productId = attributes.value("productId").toUShort();
                                    }

                                    if (attributes.hasAttribute("serialNumber"))
                                    {
                                        id.serialNumber = attributes.value("serialNumber").toString();
                                    }

                                    if (attributes.hasAttribute("vendorId"))
                                    {
                                        id.vendorId = attributes.value("vendorId").toUShort();
                                    }

                                    if (moduleID != ModuleCommands::MODULES_COUNT)
                                    {
                                        modules[moduleID] = id;
                                    }
                                }

                                xml.readNext();
                            }
                        }
                    }
                }
            }
            else
            {
                xml.raiseError(QObject::tr("The file is not an system_config version 1.0 file."));
            }
        }

        if (!xml.errorString().isEmpty())
        {
            LOG_ERROR(QObject::tr("%1\nLine %2, column %3")
                      .arg(xml.errorString())
                      .arg(xml.lineNumber())
                      .arg(xml.columnNumber()));
        }

        return !xml.error();
    }
}

///////////////////////////////////////////////////////////////
SystemState::SystemState(QObject* parent):
    //VariableController(parent),
    QObject(parent),
    mEnablesMKOKits(ModuleMKO::NO_KIT),
    mCurCommand(Q_NULLPTR),
    mMKO(Q_NULLPTR),
    mOTD(Q_NULLPTR),
    mSTM(Q_NULLPTR),
    mTech(Q_NULLPTR),
    mPowerBUP(Q_NULLPTR),
    mPowerPNA(Q_NULLPTR)
{
    mParamNames[VOLTAGE] = tr("Voltage, V");
    mParamNames[CURRENT] = tr("Current, A");
    mParamNames[TEMPERATURE] = tr("Temperature, °C");
    mParamNames[MODE] = tr("Mode");
    mParamNames[STEPS] = tr("Steps");
    mParamNames[VELOCITY] = tr("Velocity");
    mParamNames[MODE_PSY] = tr("Mode (Psy)");
    mParamNames[STEPS_PSY] = tr("Steps (Psy)");
    mParamNames[VELOCITY_PSY] = tr("Velocity (Psy)");
    mParamNames[CURRENT_PSY] = tr("Max current (Psy)");
    mParamNames[ANGLE_PSY] = tr("Angle (Psy)");
    mParamNames[MODE_NU] = tr("Mode (Nu)");
    mParamNames[STEPS_NU] = tr("Steps (Nu)");
    mParamNames[VELOCITY_NU] = tr("Velocity (Nu)");
    mParamNames[CURRENT_NU] = tr("Max current (Nu)");
    mParamNames[ANGLE_NU] = tr("Angle (Nu)");
    mParamNames[SENSOR_FLAG] = tr("Temp.Sensor Flag");

    mDefaultVariables[VOLTAGE] = "U";
    mDefaultVariables[CURRENT] = "I";
    mDefaultVariables[TEMPERATURE] = "T";
    mDefaultVariables[MODE] = "M";
    mDefaultVariables[STEPS] = "St";
    mDefaultVariables[VELOCITY] = "V";
    mDefaultVariables[MODE_PSY] = "PsyM";
    mDefaultVariables[STEPS_PSY] = "PsyS";
    mDefaultVariables[VELOCITY_PSY] = "PsyV";
    mDefaultVariables[CURRENT_PSY] = "PsyI";
    mDefaultVariables[ANGLE_PSY] = "PsyA";
    mDefaultVariables[MODE_NU] = "NuM";
    mDefaultVariables[STEPS_NU] = "NuS";
    mDefaultVariables[VELOCITY_NU] = "NuV";
    mDefaultVariables[CURRENT_NU] = "NuI";
    mDefaultVariables[ANGLE_NU] = "NuA";
    mDefaultVariables[SENSOR_FLAG] = "TSF";

    mDefaultDescriptions[VOLTAGE] = tr("Voltage, V");
    mDefaultDescriptions[CURRENT] = tr("Current, A");
    mDefaultDescriptions[TEMPERATURE] = tr("Temperature, °C");
    mDefaultDescriptions[MODE] = tr("Mode");
    mDefaultDescriptions[STEPS] = tr("Steps");
    mDefaultDescriptions[VELOCITY] = tr("Velocity");
    mDefaultDescriptions[MODE_PSY] = tr("Mode (Psy)");
    mDefaultDescriptions[STEPS_PSY] = tr("Steps (Psy)");
    mDefaultDescriptions[VELOCITY_PSY] = tr("Velocity (Psy)");
    mDefaultDescriptions[CURRENT_PSY] = tr("Max current (Psy)");
    mDefaultDescriptions[ANGLE_PSY] = tr("Angle (Psy)");
    mDefaultDescriptions[MODE_NU] = tr("Mode (Nu)");
    mDefaultDescriptions[STEPS_NU] = tr("Steps (Nu)");
    mDefaultDescriptions[VELOCITY_NU] = tr("Velocity (Nu)");
    mDefaultDescriptions[CURRENT_NU] = tr("Max current (Nu)");
    mDefaultDescriptions[ANGLE_NU] = tr("Angle (Nu)");
    mDefaultDescriptions[SENSOR_FLAG] = tr("Temperature sensor presense flag");
}

SystemState::~SystemState()
{
}

void SystemState::onApplicationStart()
{
    // Load modules configuration file
    QMap<ModuleCommands::ModuleID, COMPortModule::Identifier> modules;
    loadSystemConfig(modules);

    // Create modules objects
    mMKO = new ModuleMKO(this);
    mMKO->setModuleID(ModuleCommands::MKO);
    connect(this, SIGNAL(sendToMKO(const QMap<uint32_t,QVariant>&)), mMKO, SLOT(processCommand(const QMap<uint32_t,QVariant>&)));
    connect(mMKO, SIGNAL(commandResult(const QMap<uint32_t,QVariant>&)), this, SLOT(processResponse(const QMap<uint32_t,QVariant>&)));
    connect(mMKO, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

    mOTD = new ModuleOTD(this);
    mOTD->setId(modules.value(ModuleCommands::OTD));
    mOTD->setModuleID(ModuleCommands::OTD);
    connect(this, SIGNAL(sendToOTD(const QMap<uint32_t,QVariant>&)), mOTD, SLOT(processCommand(const QMap<uint32_t,QVariant>&)));
    connect(mOTD, SIGNAL(commandResult(const QMap<uint32_t,QVariant>&)), this, SLOT(processResponse(const QMap<uint32_t,QVariant>&)));
    connect(mOTD, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

    mSTM = new ModuleSTM(this);
    mSTM->setId(modules.value(ModuleCommands::STM));
    mSTM->setModuleID(ModuleCommands::STM);
    connect(this, SIGNAL(sendToSTM(const QMap<uint32_t,QVariant>&)), mSTM, SLOT(processCommand(const QMap<uint32_t,QVariant>&)));
    connect(mSTM, SIGNAL(commandResult(const QMap<uint32_t,QVariant>&)), this, SLOT(processResponse(const QMap<uint32_t,QVariant>&)));
    connect(mSTM, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

    mTech = new ModuleTech(this);
    mTech->setId(modules.value(ModuleCommands::TECH));
    mTech->setModuleID(ModuleCommands::TECH);
    connect(this, SIGNAL(sendToTech(const QMap<uint32_t,QVariant>&)), mTech, SLOT(processCommand(const QMap<uint32_t,QVariant>&)));
    connect(mTech, SIGNAL(commandResult(const QMap<uint32_t,QVariant>&)), this, SLOT(processResponse(const QMap<uint32_t,QVariant>&)));
    connect(mTech, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

    mPowerBUP = new ModulePower(this);
    mPowerBUP->setId(modules.value(ModuleCommands::POWER_UNIT_BUP));
    mPowerBUP->setModuleID(ModuleCommands::POWER_UNIT_BUP);
    connect(this, SIGNAL(sendToPowerUnitBUP(const QMap<uint32_t,QVariant>&)), mPowerBUP, SLOT(processCommand(const QMap<uint32_t,QVariant>&)));
    connect(mPowerBUP, SIGNAL(commandResult(const QMap<uint32_t,QVariant>&)), this, SLOT(processResponse(const QMap<uint32_t,QVariant>&)));
    connect(mPowerBUP, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

    mPowerPNA = new ModulePower(this);
    mPowerPNA->setId(modules.value(ModuleCommands::POWER_UNIT_PNA));
    mPowerPNA->setModuleID(ModuleCommands::POWER_UNIT_PNA);
    connect(this, SIGNAL(sendToPowerUnitPNA(const QMap<uint32_t,QVariant>&)), mPowerPNA, SLOT(processCommand(const QMap<uint32_t,QVariant>&)));
    connect(mPowerPNA, SIGNAL(commandResult(const QMap<uint32_t,QVariant>&)), this, SLOT(processResponse(const QMap<uint32_t,QVariant>&)));
    connect(mPowerPNA, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

    mModules[ModuleCommands::MKO] = mMKO;
    mModules[ModuleCommands::OTD] = mOTD;
    mModules[ModuleCommands::STM] = mSTM;
    mModules[ModuleCommands::TECH] = mTech;
    mModules[ModuleCommands::POWER_UNIT_BUP] = mPowerBUP;
    mModules[ModuleCommands::POWER_UNIT_PNA] = mPowerPNA;

    // Start modules initialization. They will send 'initializationFinished' signal on initialization finish depending on its internal logic
    // The entire system will be initialized and ready to execute commands after 'initializationFinished' signal will be receceived from all modules
    mPowerBUP->onApplicationStart();
    mPowerPNA->onApplicationStart();
    mOTD->onApplicationStart();
    mSTM->onApplicationStart();
    mTech->onApplicationStart();
    mMKO->onApplicationStart();

    // Create templates for internal protocol commands parameters
    setupCommandsParams();
}

void SystemState::setDefaultState()
{
    LOG_INFO("Setting default system state...");

    foreach (AbstractModule* module, mModules.values())
    {
        LOG_INFO(QString("Module %1 state is %2").arg(module->moduleName()).arg(module->moduleState()));

        if (module->moduleState() == AbstractModule::INITIALIZED_OK)
        {
            module->setDefaultState();
        }
    }
}

int SystemState::simpltst1(int z)
{
    /*
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
    }*/

    return z ;
}

void SystemState::MKO_data(QString data)
{
    //ui->label_MKO->setText(data);
}

void SystemState::MKO_cm_data(QString data)
{
    /*
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

    if(mEnablesMKOKits == ModuleMKO::ALL_KITS)
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
    */
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

    //if (er1 == 1) ui->err1->setText("Overvoltage protection!"); //TODO - ошибки установки на блоке питания, если 0 - ошибки нет
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

void SystemState::on_MKO_osn_clicked()
{
    /*mEnablesMKOKits ^= ModuleMKO::MAIN_KIT;

    if(mEnablesMKOKits & ModuleMKO::MAIN_KIT)
    {
        //ui->MKO_osn->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else
    {
        //ui->MKO_osn->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }

    connect(this, SIGNAL(MKO_ch(int)), mMKO, SLOT(MKO_chan(int)));
    emit MKO_ch(mEnablesMKOKits);
    disconnect(this, SIGNAL(MKO_ch(int)), mMKO, SLOT(MKO_chan(int)));*/
}

void SystemState::on_MKO_rez_clicked()
{
    /*mEnablesMKOKits ^= ModuleMKO::RESERVE_KIT;
    if(mEnablesMKOKits & ModuleMKO::RESERVE_KIT)
    {
        //ui->MKO_rez->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    }
    else
    {
        //ui->MKO_rez->setStyleSheet(QString::fromUtf8("background-color: rgb(230, 230, 230);"));
    }

    connect(this, SIGNAL(MKO_ch(int)), mMKO, SLOT(MKO_chan(int)));
    emit MKO_ch(mEnablesMKOKits);
    disconnect(this, SIGNAL(MKO_ch(int)), mMKO, SLOT(MKO_chan(int)));*/
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

void SystemState::setupCommandsParams()
{
    createPowerUnitCommandsParams();

    int TODO; // replace by constant usage

    {
        QMap<int, QStringList> inParams;
        QMap<int, QStringList> outParams;

        // commands input params
        QStringList sendTestArrayParams; // hardcoded with 0
        QStringList receiveTestArrayParams; // hardcoded with 0 checks on receive
        QStringList sendTestArrayForChannelParams; // hardcoded with 0
        QStringList receiveTestArrayForChannelParams; // hardcoded with 0 checks on receive
        QStringList enablePowerSupplyForAngleSensorParams; // auto set params (main of reserve kit)

        QStringList sendCommandArrayParams;
        sendCommandArrayParams.push_back(paramName(MODE_PSY));
        sendCommandArrayParams.push_back(paramName(STEPS_PSY));
        sendCommandArrayParams.push_back(paramName(VELOCITY_PSY));
        sendCommandArrayParams.push_back(paramName(CURRENT_PSY));
        sendCommandArrayParams.push_back(paramName(MODE_NU));
        sendCommandArrayParams.push_back(paramName(STEPS_NU));
        sendCommandArrayParams.push_back(paramName(VELOCITY_NU));
        sendCommandArrayParams.push_back(paramName(CURRENT_NU));

        QStringList sendCommandArrayForChannelParams;
        sendCommandArrayForChannelParams.push_back(paramName(MODE));
        sendCommandArrayForChannelParams.push_back(paramName(STEPS));
        sendCommandArrayForChannelParams.push_back(paramName(VELOCITY));
        sendCommandArrayForChannelParams.push_back(paramName(CURRENT));

        QStringList receiveCommandArrayParams;
        receiveCommandArrayParams.push_back(paramName(MODE_PSY));
        receiveCommandArrayParams.push_back(paramName(STEPS_PSY));
        receiveCommandArrayParams.push_back(paramName(VELOCITY_PSY));
        receiveCommandArrayParams.push_back(paramName(CURRENT_PSY));
        receiveCommandArrayParams.push_back(paramName(MODE_NU));
        receiveCommandArrayParams.push_back(paramName(STEPS_NU));
        receiveCommandArrayParams.push_back(paramName(VELOCITY_NU));
        receiveCommandArrayParams.push_back(paramName(CURRENT_NU));
        receiveCommandArrayParams.push_back(paramName(ANGLE_PSY));
        receiveCommandArrayParams.push_back(paramName(ANGLE_NU));
        receiveCommandArrayParams.push_back(paramName(SENSOR_FLAG));
        receiveCommandArrayParams.push_back(paramName(TEMPERATURE));

        QStringList receiveCommandArrayForChannelParams;
        sendCommandArrayForChannelParams.push_back(paramName(MODE));
        sendCommandArrayForChannelParams.push_back(paramName(STEPS));
        sendCommandArrayForChannelParams.push_back(paramName(VELOCITY));
        sendCommandArrayForChannelParams.push_back(paramName(CURRENT));

        inParams[ModuleMKO::SEND_TEST_ARRAY] = sendTestArrayParams;
        inParams[ModuleMKO::SEND_COMMAND_ARRAY] = sendCommandArrayParams;
        inParams[ModuleMKO::SEND_TEST_ARRAY_FOR_CHANNEL] = sendTestArrayForChannelParams;
        inParams[ModuleMKO::SEND_COMMAND_ARRAY_FOR_CHANNEL] = sendCommandArrayForChannelParams;
        inParams[ModuleMKO::SEND_TO_ANGLE_SENSOR] = enablePowerSupplyForAngleSensorParams;

        outParams[ModuleMKO::RECEIVE_TEST_ARRAY] = receiveTestArrayParams;
        outParams[ModuleMKO::RECEIVE_COMMAND_ARRAY] = receiveCommandArrayParams;
        outParams[ModuleMKO::RECEIVE_TEST_ARRAY_FOR_CHANNEL] = receiveTestArrayForChannelParams;
        outParams[ModuleMKO::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL] = receiveCommandArrayForChannelParams;

        mInParams[ModuleCommands::MKO] = inParams;
        mOutParams[ModuleCommands::MKO] = outParams;
    }

    // STM
    {
        //QMap<int, QStringList> params;
        //mInParams[ModuleCommands::STM] = params;

        // STM
        //addCommand(tr("Включить канал СТМ к БП"), ModuleCommands::POWER_CHANNEL_CTRL);
        //addCommand(tr("Проверить предохранители"), ModuleCommands::GET_PWR_MODULE_FUSE_STATE);
        //addCommand(tr("Получить телеметрию канала"), ModuleCommands::GET_CHANNEL_TELEMETRY);
        //addCommand(tr("Включить канал СТМ к МКО"), ModuleCommands::SET_MKO_PWR_CHANNEL_STATE);
        //addCommand(tr("Получить состояние канала СТМ к БП"), ModuleCommands::GET_POWER_MODULE_STATE);
        //addCommand(tr("Получить состояние канала СТМ к МКО"), ModuleCommands::GET_MKO_MODULE_STATE);

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

void SystemState::createPowerUnitCommandsParams()
{
    QStringList powerParams;
    powerParams.push_back(paramName(VOLTAGE));
    powerParams.push_back(paramName(CURRENT));

    // commands input params
    QMap<int, QStringList> params;


    QStringList powerSetParams;
    powerSetParams.push_back(paramName(VOLTAGE));
    params[ModuleCommands::SET_VOLTAGE_AND_CURRENT] = powerSetParams;
    params[ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT] = powerParams;
    mInParams[ModuleCommands::POWER_UNIT_BUP] = params;
    mInParams[ModuleCommands::POWER_UNIT_PNA] = params;

    params.clear();
    params[ModuleCommands::GET_VOLTAGE_AND_CURRENT] = powerParams;
    mOutParams[ModuleCommands::POWER_UNIT_BUP] = params;
    mOutParams[ModuleCommands::POWER_UNIT_PNA] = params;
}

void SystemState::createOTDCommandsParams()
{
    QMap<int, QStringList> params;

    QStringList temperatureParams;

    // PT-100 params
    temperatureParams.clear();
    int ptCount = mOTD->ptCount();
    for (int i = 0; i < ptCount; ++i)
    {
        temperatureParams.push_back(/*QString::number(i + 1) + QString(". ") + */paramName(TEMPERATURE));
    }

    params[ModuleCommands::GET_TEMPERATURE_PT100] = temperatureParams;

    // DS1820 line 1 params
    temperatureParams.clear();
    int dsCount1 = mOTD->dsCount(ModuleOTD::PSY);
    for (int i = 0; i < dsCount1; ++i)
    {
        temperatureParams.push_back(/*QString::number(i + 1) + QString(". ") +*/ paramName(TEMPERATURE));
    }

    params[ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1] = temperatureParams;

    // DS1820 line 2 params
    temperatureParams.clear();
    int dsCount2 = mOTD->dsCount(ModuleOTD::NU);
    for (int i = 0; i < dsCount2; ++i)
    {
        temperatureParams.push_back(/*QString::number(i + 1) + QString(". ") +*/ paramName(TEMPERATURE));
    }

    params[ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2] = temperatureParams;

    mOutParams[ModuleCommands::OTD] = params;
}

void SystemState::sendCommand(CmdActionModule* command)
{
    mCurCommand = command;

    const QMap<QString, QVariant>& inputParams = command->inputParams();
    const QMap<QString, QVariant>& outputParams = command->outputParams();
    const QList<int>& implicitInputParams = command->implicitParams();

    VariableController* vc = command->variableController();

    QMap<uint32_t, QVariant> params;
    params[MODULE_ID] = QVariant(uint32_t(command->module()));
    params[COMMAND_ID] = QVariant(uint32_t(command->operation()));
    params[INPUT_PARAMS_COUNT] = QVariant(uint32_t(inputParams.size()));
    params[OUTPUT_PARAMS_COUNT] = QVariant(uint32_t(outputParams.size()));
    params[IMPLICIT_PARAMS_COUNT] = QVariant(uint32_t(implicitInputParams.size()));

    int TODO; // start "waiting for execution" protection timer

    // input params -> [type, value]

    uint32_t i = 0;
    for (auto it = inputParams.begin(); it != inputParams.end(); ++it)
    {
        params[INPUT_PARAM_BASE + i] = QVariant(uint32_t(paramID(it.key())));
        ++i;

        if (it.value().type() == QMetaType::QString)
        {
            qreal value = vc->currentValue(it.value().toString());
            params[INPUT_PARAM_BASE + i] = QVariant(value);
        }
        else
        {
            params[INPUT_PARAM_BASE + i] = it.value();
        }

        ++i;
    }

    // output params -> [type, vaiableName]
    i = 0;
    for (auto it = outputParams.begin(); it != outputParams.end(); ++it)
    {
        params[OUTPUT_PARAM_BASE + i] = QVariant(uint32_t(paramID(it.key())));
        ++i;

        params[OUTPUT_PARAM_BASE + i] = QVariant(it.value());
        ++i;
    }

    // implicit input params
    i = 0;
    for (auto it = implicitInputParams.begin(); it != implicitInputParams.end(); ++it)
    {
        params[IMPLICIT_PARAM_BASE + i] = QVariant(*it);
        ++i;
    }

    switch (command->module())
    {
    case ModuleCommands::POWER_UNIT_BUP:
        emit sendToPowerUnitBUP(params);
        break;
    case ModuleCommands::POWER_UNIT_PNA:
        emit sendToPowerUnitPNA(params);
        break;
    case ModuleCommands::OTD:
        emit sendToOTD(params);
        break;
    case ModuleCommands::STM:
        emit sendToSTM(params);
        break;
    case ModuleCommands::MKO:
        emit sendToMKO(params);
        break;
    case ModuleCommands::TECH:
        emit sendToTech(params);
        break;

    default:
        break;
    }
}

void SystemState::onExecutionFinished(uint32_t error)
{
    if (error != 0) // 0 - successful execution
    {
        QMetaEnum moduleEnum = QMetaEnum::fromType<ModuleCommands::ModuleID>();
        QMetaEnum commandEnum = QMetaEnum::fromType<ModuleCommands::CommandID>();

        LOG_ERROR("Command execution failed. Module:%s Command:%s Error:%s",
                  moduleEnum.valueToKey(mCurCommand->module()),
                  commandEnum.valueToKey(mCurCommand->operation()),
                  QString::number(error));
    }

    mCurCommand = Q_NULLPTR;
    emit commandFinished(error == 0);
}

void SystemState::processResponse(const QMap<uint32_t, QVariant>& response)
{
    if (!mCurCommand)
    {
        LOG_ERROR("Unexpected response received");
        return;
    }

    int TODO; // stop "waiting for execution" protection timer

    VariableController* vc = mCurCommand->variableController();
    uint32_t paramsCount = response.value(OUTPUT_PARAMS_COUNT).toUInt();

    QString varName;
    qreal value;

    for (uint32_t i = 0; i < paramsCount; ++i)
    {
        if (i % 2 == 0)
        {
            varName = response.value(OUTPUT_PARAM_BASE + i).toString();
        }
        else
        {
            value = response.value(OUTPUT_PARAM_BASE + i).toDouble();
            vc->setCurrentValue(varName, value);
        }
    }

    if (paramsCount > 0)
    {
        vc->makeDataSnapshot();
    }

    uint32_t error = response.value(ERROR_CODE, QVariant(uint32_t(0))).toUInt();

    // in case of power unit
    //if (error == 1) ui->err1->setText("Overvoltage protection!"); //TODO - ошибки установки на блоке питания, если 0 - ошибки нет
    //if (error == 2) ui->err1->setText("Overcurrent protection!");
    //if (error == 4) ui->err1->setText("Overpower protection!");
    //if (error == 8) ui->err1->setText("Overtemperature protection!");

    onExecutionFinished(error);
}

QString SystemState::paramName(ParamID param) const
{
    return mParamNames.value(param, "");
}

QString SystemState::paramDefaultVarName(ParamID param) const
{
    return mDefaultVariables.value(param, "");
}

QString SystemState::paramDefaultDesc(ParamID param) const
{
    return mDefaultDescriptions.value(param, "");
}

SystemState::ParamID SystemState::paramID(const QString& name) const
{
    return mParamNames.key(name, UNDEFINED);
}

void SystemState::onModuleStateChanged(ModuleCommands::ModuleID id, AbstractModule::ModuleState from, AbstractModule::ModuleState to)
{
    QMetaEnum stateEnum = QMetaEnum::fromType<AbstractModule::ModuleState>();
    AbstractModule* module = mModules.value(id);

    if (from == AbstractModule::INITIALIZING)
    {
        if (to == AbstractModule::INITIALIZED_OK)
        {
            LOG_INFO(QString("%1 initializion SUCCESS").arg(module->moduleName()));
            if (id == ModuleCommands::OTD)
            {
                createOTDCommandsParams();
            }
        }
        else if (to == AbstractModule::INITIALIZED_FAILED)
        {
            LOG_ERROR(QString("%1 initialization FAILED! Error: %2").arg(module->moduleName()).arg(module->errorString()));
        }
        else
        {
            LOG_ERROR(QString("Unexpected %1 state changing from %2 to %3").arg(module->moduleName()).arg(stateEnum.valueToKey(from)).arg(stateEnum.valueToKey(to)));
        }

        // check all modules are started
        bool allModulesStarted = true;
        foreach (AbstractModule* m, mModules.values())
        {
            if (m->moduleState() == AbstractModule::INITIALIZED_FAILED || m->moduleState() == AbstractModule::INITIALIZED_OK)
            {
                continue;
            }

            allModulesStarted = false;
            break;
        }

        // if all modules are started, set them to default state
        if (allModulesStarted)
        {
            setDefaultState();
        }
    }
}

void SystemState::onCyclogramStart()
{
    mMKO->onCyclogramStart();
}
