#ifndef MODULE_TECH_H
#define MODULE_TECH_H

#include "Headers/system/okb_module.h"

class ModuleTech: public ModuleOKB
{
    Q_OBJECT

public:
    enum InterfaceID
    {
        CAN,
        RS485
    };

    ModuleTech(QObject* parent);
    ~ModuleTech();

    int tech_send(int com, int x, int y);
    int tech_read(int x);
    QString tech_read_buf(int x, int len);

public slots:
    void processCustomCommand(const Transaction& request, Transaction& response) override;
//    void onApplicationFinish() override;
//    void setDefaultState() override;

protected:
    void onModuleError() override;
    bool processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response) override;

private:
    void statusRS();
    void statusCAN();
    void send_tech_1();
    void send_tech_2();
};

#endif // MODULE_TECH_H
