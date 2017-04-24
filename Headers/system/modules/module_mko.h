#ifndef MODULE_MKO_H
#define MODULE_MKO_H

#include "Headers/system/abstract_module.h"

class QTimer;

class ModuleMKO: public AbstractModule
{
    Q_OBJECT

public:

    enum KitID
    {
        NO_KIT      = 0x00,
        MAIN_KIT    = 0x01,
        RESERVE_KIT = 0x02,
        ALL_KITS = MAIN_KIT | RESERVE_KIT
    };

    enum CommandID
    {
        SEND_TEST_ARRAY,
        RECEIVE_TEST_ARRAY,
        SEND_COMMAND_ARRAY,
        RECEIVE_COMMAND_ARRAY,
        SEND_TEST_ARRAY_FOR_CHANNEL,
        RECEIVE_TEST_ARRAY_FOR_CHANNEL,
        SEND_COMMAND_ARRAY_FOR_CHANNEL,
        RECEIVE_COMMAND_ARRAY_FOR_CHANNEL,
        SEND_TO_ANGLE_SENSOR,

        // custom
        START_MKO,
        STOP_MKO
    };

    Q_ENUM(CommandID)

    enum Subaddress
    {
        ANGLE_SENSOR_SUBADDRESS = 0x06,
        PSY_CHANNEL_SUBADDRESS  = 0x01,
        NU_CHANNEL_SUBADDRESS   = 0x02,
        RECEIVE_SUBADDRESS      = 0x0D,
        SEND_SUBADDRESS         = 0x0C
    };

    enum AngleSensorPowerSupplySource
    {
        PS_FROM_MAIN_KIT    = 0x20,
        PS_FROM_RESERVE_KIT = 0x40
    };

    ModuleMKO(QObject* parent);
    ~ModuleMKO();

public slots:
    void processCommand(const QMap<uint32_t, QVariant>& params) override;

    void onApplicationStart() override;
    void setDefaultState() override;
    void onCyclogramStart(); //TODO remove (ваще какая-то шляпа, из-за которой вероятно все проблемы и есть)

private slots:
    void readResponse();
    void sendResponse();

    void sendDataToBUP(uint16_t address, uint16_t subaddress, uint16_t* data, uint16_t wordsCount);
    void requestDataFromBUP(uint16_t address, uint16_t subaddress, uint16_t expectedWordsInResponse);

    void processResponseWord(uint16_t responseWord, QStringList& errors);

    // TODO refactor
    void startMKO();
    void startMKO1();
    void stopMKO();
    void stopMKO1();

private:
    struct AxisData
    {
        int16_t mode;
        int32_t steps;
        int16_t velocity;
        int16_t current;
    };

    // КС - "командное слово" - двухбайтовая "шапка" операции, мы отправляем на БУП
    // ОС - "ответное слово" - двухбайтова шапка операции, мы ее получаем с БУПа; получая его мы парсим на предмет ошибок операции

    // 2.1.1 Операция обмена при выдаче из БКУ в БУП НА тестового массива с использованием формата 1
    // (используется для тестирования БУП НА при включении только основного или только резервного полукомплекта БУП НА). Используется подадрес 12 (запись)
    void sendTestArray(uint16_t address); // мы отправляем на БУП КС "Прими тестовый массив" + 11 слов данных, в ответ получаем ОС

    // 2.1.2 Операция обмена при чтении из ОУ БУП НА по запросу БКУ тестового массива по формату 2
    // (тестирование БУП НА при включении только основного или только резервного полукомплекта БУП НА). Используется подадрес 12 (чтение)
    void receiveTestArray(uint16_t address); // мы отправляем на БУП КС "Дай тестовый массив", нам в ответ прилетает ОС + 11 слов данных

    // 2.1.3 Операция обмена при выдаче из БКУ в ОУ БУП НА командного массива на управление обоими модулями (ψ и υ) с использованием формата 1
    // (при включенном только основном или только резервном полукомплекте БУП НА). Используется подадрес 12
    void sendCommandArray(uint16_t address, const AxisData& psy, const AxisData& nu); // мы отправляем КС "Прими командный массив" + 11 слов данных (5 на каждую ось + контрольная сумма)

    // 2.1.4 Операция обмена при чтении из ОУ БУП НА по запросу БКУ контрольного массива по обоим модулям (ψ и υ) по формату 2
    // (при включенном только основном или только резервном полукомплекте БУП НА). Используется подадрес 13
    void receiveCommandArray(uint16_t address); // мы отправляем КА "Дай командный массив", нам в ответ прилетает ОС + 21 слово данных

    //2.1.5 Операци я обмена при выдаче из БКУ в ОУ БУП НА тестового массива с использованием формата 1
    //(тестирование БУП НА при включении обоих полукомплектов БУП НА).
    //Используется подадрес 01 (запись) – тестирование канала ψ и подадрес 02 (запись) – тестирование канала υ.
    void sendTestArrayForChannel(uint16_t address, Subaddress channel); // Почти то же самое, что sendTestArray(), только для одной оси (ψ или υ): туда КС + 5 слов + контрольная сумма, оттуда ОС

    //2.1.6 Операция обмена при чтении из ОУ БУП НА по запросу БКУ тестового массива по формату 2 (тестирование БУП НА при включении обоих полукомплектов БУП НА).
    // Используется подадрес 01 (чтение) – тестирование канала ψ и подадрес 02 (чтение) – тестирование канала υ.
    void receiveTestArrayForChannel(uint16_t address, Subaddress channel); // Почти то же самое, что receiveTestArray(), только для одной оси (ψ или υ): туда КС, оттуда ОС + 5 слов + контрольная сумма

    //2.1.7 Операция обмена при выдаче из БКУ в ОУ БУП НА командного массива на управление модулем ψ (модулем υ) с использованием формата 1
    //(управление при включении обоих полукомплектов БУП НА). Используется подадрес 01 (02)
    void sendCommandArrayForChannel(uint16_t address, Subaddress channel, const AxisData& axisData); // Почти то же самое, что sendCommandArray(), только для одной оси (ψ или υ): туда КС + 5 слов + контрольная сумма, оттуда ОС

    //2.1.8 Операция обмена при чтении из ОУ БУП НА по запросу БКУ контрольного массива по ψ и υ по формату 2
    //(управление при включении обоих полукомплектов БУП НА).  Используется подадрес 13.
    void receiveCommandArrayForChannel(uint16_t address, Subaddress channel); // Почти то же самое, что receiveCommandArray(), только для одной оси (ψ или υ): туда КС, оттуда ОС + 5 слов + контрольная сумма

    //2.1.9 Операция обмена при выдаче из БКУ в ОУ БУП НА массива для подачи питания на ДУ с использованием формата 1. Используется подадрес 06
    void sendAngleSensorData(uint16_t address);

    bool mMainKitEnabled;
    bool mReserveKitEnabled;

    //TODO remove
    uint16_t mWordsToReceive;
    uint16_t mWordsSent;
    KitID mActiveKits = NO_KIT;

    int mRepeatedRequests;
};

#endif // MODULE_MKO_H
