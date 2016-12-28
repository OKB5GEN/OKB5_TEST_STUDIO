#include "Headers/system/modules/module_otd.h"

ModuleOTD::ModuleOTD(QObject* parent):
    Module(parent)
{
}

ModuleOTD::~ModuleOTD()
{

}

void ModuleOTD::resetLine(LineID line)
{
    ModuleCommands::CommandID command = (line == PSY) ? ModuleCommands::RESET_LINE_1 : ModuleCommands::RESET_LINE_2;
    if (!send(command))
    {
        int TODO;
        //emit err_OTD("Ошибка при перезагрузке!");
    }
}

void ModuleOTD::postInit()
{
    // 1. Read sensors count on both lines
    // 2. Read sensors addresses on both lines
    // 3. Start measurement on both lines
    // 4. Get temperature on all sensors on both lines
    // 5. Get PT-100 temperature value
    // 6. Set received data to system state?
    // 7. Launch update timer
}
