#ifndef MODULE_STM_H
#define MODULE_STM_H

#include "Headers/system/okb_module.h"

class ModuleSTM: public ModuleOKB
{
    Q_OBJECT

public:
    ModuleSTM(QObject* parent);
    ~ModuleSTM();

    int setPowerChannelState(int channel, ModuleCommands::PowerState state);// 1-3 POW_ANT_DRV_CTRL channels (1 and 2 used as main and reserve), 4-6 channels
    double stm_data_ch(int ch);

    int stm_on_mko(int y, int x);
    int stm_check_fuse(int fuse);

private:
};

#endif // MODULE_STM_H
