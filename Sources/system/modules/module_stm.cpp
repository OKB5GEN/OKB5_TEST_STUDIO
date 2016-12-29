#include "Headers/system/modules/module_stm.h"

namespace
{
    static const int STM_DEFAULT_ADDR = 0x22;
}

ModuleSTM::ModuleSTM(QObject* parent):
    ModuleOKB(parent)
{
}

ModuleSTM::~ModuleSTM()
{

}

int ModuleSTM::setPowerChannelState(int channel, ModuleCommands::PowerState state)
{
    QByteArray request(4, 0);
    request[0] = STM_DEFAULT_ADDR;
    request[1] = ModuleCommands::POWER_CHANNEL_CTRL;
    request[2] = channel;
    request[3] = (state == ModuleCommands::POWER_ON) ? 1 : 0;

    QByteArray response;
    COMPortModule::send(request, response);
    return response[3];

    int TODO; // следить за состоянием подключения - в исходном было, что если питание подано/отключено, то ставим флажок подключено/отключено
    /*
     *     if(setPowerChannelState(1, POWER_ON) == 1 && m_flag_con1 == 0)
    {
        m_flag_con1 = 1;
    }
    else if(setPowerChannelState(1, POWER_OFF) == 1 && m_flag_con1 == 1)
    {
        m_flag_con1 = 0;
    }
*/
}

double ModuleSTM::stm_data_ch(int ch)
{
    int TODO;

    return 0;

    //if (isActive(STM))
    //{
    //    QByteArray buffer(4, 0);
    //    buffer[0] = STM_DEFAULT_ADDR;
    //    buffer[1] = ModuleCommands::GET_CHANNEL_TELEMETRY;
    //    buffer[2] = ch;
    //    buffer[3] = 0x00;

    //    QByteArray readData1 = send(getPort(STM), buffer);

    //    uint8_t uu1, uu2;
    //    uu1 = readData1[2];
    //    uu2 = readData1[3];
    //    double res = (uu1 << 8) | uu2;
    //    return res;
    //}

    //return 50000;


    /*
     *     double res = stm_data_ch(15)/10000;
    if(res >= 0.5)
    {
        //ui->pushButton_ctm_ch_15->setText ("Разъединена");
    }
    else if(res >= 0 && res < 0.51 && req_stm()=="")
    {
        //ui->pushButton_ctm_ch_15->setText ("Соединена");
    }

    все остальные /10000

    double res = stm_data_ch(n)/10000;
*/
}

int ModuleSTM::stm_on_mko(int x, int y)
{
    int TODO;
    //QByteArray buffer(4, 0);
    //buffer[0] = STM_DEFAULT_ADDR;
    //buffer[1] = ModuleCommands::SET_MKO_PWR_CHANNEL_STATE;
    //buffer[2] = x;
    //buffer[3] = y;

    //QByteArray readData1 = send(getPort(STM), buffer);
    //return readData1[3];

    return 0;
}

int ModuleSTM::stm_check_fuse(int fuse)
{
    int TODO;

    //QByteArray buffer(4, 0);
    //buffer[0] = STM_DEFAULT_ADDR;
    //buffer[1] = ModuleCommands::GET_PWR_MODULE_FUSE_STATE;
    //buffer[2] = fuse;
    //buffer[3] = 0x00;
    //QByteArray readData1 = send(getPort(STM), buffer);
    //return readData1[3];

    return 0;

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
