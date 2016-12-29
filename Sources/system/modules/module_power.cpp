#include "Headers/system/modules/module_power.h"

namespace
{
    static const uint8_t MAX_VOLTAGE = 42; // volts
    static const uint8_t MAX_CURRENT = 10; // ampers
    static const uint8_t MAX_POWER = 155; // watts actually 160, but for safety purposes reduced to 155
}

ModulePower::ModulePower(QObject* parent):
    COMPortModule(parent),
    mState(ModuleCommands::POWER_OFF)
{
    int TODO; // проверять перед установкой на максимум, ограничения ставим сразу, текущее значение, ограничиваем по максимуму
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

ModulePower::~ModulePower()
{

}

bool ModulePower::postInit()
{
    return true;
}

void ModulePower::resetError()
{
    QByteArray request(7, 0);

    request[0] = 0xf1;
    request[1] = 0x00;
    request[2] = 0x36;
    request[3] = 0x0a;
    request[4] = 0x0a;
    request[5] = 0x01;
    request[6] = 0x3b;

    QByteArray response;
    send(request, response);
}

void ModulePower::startPower()
{
    // PowerON
    QByteArray request1(7, 0);
    request1[0] = 0xf1;
    request1[1] = 0x00;
    request1[2] = 0x36;
    request1[3] = 0x10;
    request1[4] = 0x10;
    request1[5] = 0x01;
    request1[6] = 0x47;

    QByteArray response1;
    send(request1, response1);

    QByteArray request2(7, 0);
    request2[0] = 0xf1;//power off
    request2[1] = 0x00;
    request2[2] = 0x36;
    request2[3] = 0x01;
    request2[4] = 0x00;
    request2[5] = 0x01;
    request2[6] = 0x28;

    QByteArray response2;
    send(request1, response2);

    setVoltageAndCurrent(0.5);
}

void ModulePower::setPowerState(ModuleCommands::PowerState state)
{
    int TODO; // remove parameter

    if (mState == ModuleCommands::POWER_ON)
    {

    }

    QByteArray request(7, 0);
    request[0] = 0xf1;//power on/off
    request[1] = 0x00;
    request[2] = 0x36;
    request[3] = 0x01;
    request[4] = 0x01;
    request[5] = 0x01;
    request[6] = (state == ModuleCommands::POWER_ON) ? 0x29 : 0x28;

    QByteArray response;
    send(request, response);

    mState = state;
}

void ModulePower::setPowerValue(uint8_t valueID, double value, double maxValue)
{
    QByteArray request(7, 0);
    uint32_t val = uint32_t((value * 256 * 100) / maxValue);

    if(val > (256 * 100))
    {
        val = (256 * 100);
    }

    request[0] = 0xf1;
    request[1] = 0x00;
    request[2] = valueID;
    request[3] = (val >> 8) & 0xFF;
    request[4] = val & 0xFF;
    uint16_t sum = 0;
    for(int i = 0; i < 5; i++)
    {
        uint8_t s = request[i];
        sum = (sum + s) & 0xFFFF;
    }

    request[5] = ((sum >> 8) & 0xFF);
    request[6] = (sum & 0xFF);
    QByteArray response;
    send(request, response);
}

void ModulePower::setMaxVoltageAndCurrent(double voltage, double current)
{
    setPowerValue(MAX_VOLTAGE_VAL, voltage, MAX_VOLTAGE);
    setPowerValue(MAX_CURRENT_VAL, current, MAX_CURRENT);
}

void ModulePower::setVoltageAndCurrent(double voltage)
{
    setPowerValue(CUR_VOLTAGE_VAL, voltage, MAX_VOLTAGE);
    setPowerValue(CUR_CURRENT_VAL, ((double)MAX_POWER) / voltage, MAX_CURRENT);
}

void ModulePower::getCurVoltageAndCurrent(double& voltage, double& current, uint8_t& error)
{
    QByteArray request(5, 0);
    request[0] = 0x75;
    request[1] = 0x00;
    request[2] = 0x47;
    request[3] = 0x00;
    request[4] = 0xbc;

    QByteArray response;
    send(request, response);

    uint8_t uu1, uu2;
    error = (response[4] >> 4);

    uu1 = response[5];
    uu2 = response[6];
    voltage = (uu1 << 8) | uu2;
    voltage = voltage * MAX_VOLTAGE / 256;

    uu1 = response[7];
    uu2 = response[8];
    current = (uu1 << 8) | uu2;
    current = current * MAX_CURRENT / 256;
}
