#include "Headers/system/modules/module_tech.h"

namespace
{
    static const uint8_t TECH_DEFAULT_ADDR = 0x56;
    static const int WAIT_FOR_RESPONSE_TIME = 100; // msec
}

ModuleTech::ModuleTech(QObject* parent):
    ModuleOKB(parent)
{
    int TODO; // хз чего делать с данными по CAN-RS485 и нужно ли это вообще в циклограмме
    // еще хз чего делать с одновременным запуском Сашиной проги и моей, будт ли чего вообще доступно по COM-портам из двух приложений?:)

}

ModuleTech::~ModuleTech()
{

}

int ModuleTech::tech_send(int com, int x, int y)
{
    int TODO;

    /*QByteArray request;
    request.append(TECH_DEFAULT_ADDR);
    request.append(com);
    request.append(x);
    request.append(y);

    QByteArray response;
    COMPortModule::send(request, response, WAIT_FOR_RESPONSE_TIME);
    return response[3];*/

    return 0;

}

int ModuleTech::tech_read(int x)
{
    int TODO;

    return 0;

    /*if(isActive(TECH))
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

    return 0;*/
}

QString ModuleTech::tech_read_buf(int x, int len)
{
    int TODO;
    return "";

    /*uint8_t command = 0;
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

    return result;*/
}

void ModuleTech::statusRS()
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

void ModuleTech::statusCAN()
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

void ModuleTech::send_tech_1()
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
                    //ui->tech_error->setText(" Oшибка при формировании посылки данных!");
                }
            }
            else
            {
                i=s;
                ersend=2;
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

void ModuleTech::send_tech_2()
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

void ModuleTech::processCustomCommand(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
{
    int TODO; // do not send signals, just execute command and fill response
}

bool ModuleTech::processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response)
{
    return true;
    int TODO;
}

void ModuleTech::onApplicationFinish()
{
    int TODO;
}

void ModuleTech::onModuleError()
{
    int TODO; //TODO here will be processing
}

void ModuleTech::setDefaultState()
{
    setModuleState(AbstractModule::SETTING_TO_SAFE_STATE);
    setModuleState(AbstractModule::SAFE_STATE);
    int TODO;
}
