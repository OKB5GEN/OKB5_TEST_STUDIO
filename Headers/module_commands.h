#ifndef MODULE_COMMANDS_H
#define MODULE_COMMANDS_H

#include <QObject>

class ModuleCommands: public QObject
{
    Q_OBJECT

public:
    enum CommandID
    {
        // Own modules commands (documented) >>>>>
        GET_MODULE_ADDRESS                = 0x01,
        GET_STATUS_WORD                   = 0x02,
        RESET_ERROR                       = 0x03,
        SOFT_RESET                        = 0x04,
        //RESERVED_0x05                     = 0x05,
        GET_SOWFTWARE_VERSION             = 0x06,
        ECHO                              = 0x07,
        //RESERVED_0x08                     = 0x08,
        //RESERVED_0x09                     = 0x09,
        //RESERVED_0x0A                     = 0x0A,
        SET_POWER_CHANNEL_STATE           = 0x0B, // Can be sent to STM only
        GET_FUSE_STATE                    = 0x0C, // Can be sent to STM only
        GET_CHANNEL_TELEMETRY             = 0x0D, // Can be sent to STM only
        SET_MKO_POWER_CHANNEL_STATE       = 0x0E, // Can be sent to STM only
        //MATRIX_CMD_CTRL                   = 0x0F, // Can be sent to MKU only
        SET_PACKET_SIZE_CAN               = 0x10, // Can be sent to TECH only
        ADD_BYTES_CAN                     = 0x11, // Can be sent to TECH only
        SEND_PACKET_CAN                   = 0x12, // Can be sent to TECH only
        CHECK_RECV_DATA_CAN               = 0x13, // Can be sent to TECH only
        RECV_DATA_CAN                     = 0x14, // Can be sent to TECH only
        CLEAN_BUFFER_CAN                  = 0x15, // Can be sent to TECH only
        SET_PACKET_SIZE_RS485             = 0x16, // Can be sent to TECH only
        ADD_BYTES_RS485                   = 0x17, // Can be sent to TECH only
        SEND_PACKET_RS485                 = 0x18, // Can be sent to TECH only
        CHECK_RECV_DATA_RS485             = 0x19, // Can be sent to TECH only
        RECV_DATA_RS485                   = 0x1A, // Can be sent to TECH only
        CLEAN_BUFFER_RS485                = 0x1B, // Can be sent to TECH only
        GET_TEMPERATURE_PT100             = 0x1C, // Can be sent to OTD only
        GET_DS1820_COUNT_LINE_1           = 0x1D, // Can be sent to OTD only (Psi)
        GET_DS1820_COUNT_LINE_2           = 0x1E, // Can be sent to OTD only (Nu)
        GET_TEMPERATURE_DS1820_LINE_1     = 0x1F, // Can be sent to OTD only (Psi)
        GET_TEMPERATURE_DS1820_LINE_2     = 0x20, // Can be sent to OTD only (Nu)
        GET_POWER_CHANNEL_STATE           = 0x21, // Can be sent to STM only
        //GET_MKU_MODULE_STATE              = 0x22, // Can be sent to MKU only
        GET_MKO_POWER_CHANNEL_STATE       = 0x23, // Can be sent to STM only
        SET_MODE_RS485                    = 0x24, // Can be sent to TECH only
        SET_SPEED_RS485                   = 0x25, // Can be sent to TECH only
        RESET_LINE_1                      = 0x26, // Can be sent to OTD only (Psi)
        RESET_LINE_2                      = 0x27, // Can be sent to OTD only (Nu)
        START_MEASUREMENT_LINE_1          = 0x28, // Can be sent to OTD only (Psi) 1-2 seconds to perform
        START_MEASUREMENT_LINE_2          = 0x29, // Can be sent to OTD only (Nu) 1-2 seconds to perform
        GET_DS1820_ADDR_LINE_1            = 0x2A, // Can be sent to OTD only (Psi)
        GET_DS1820_ADDR_LINE_2            = 0x2B, // Can be sent to OTD only (Nu)
        SET_TECH_INTERFACE                = 0x3B, // Can be sent to TECH only
        GET_TECH_INTERFACE                = 0x3C, // Can be sent to TECH only
        RECV_DATA_SSI                     = 0x3D, // Can be sent to TECH only

        // Third party modules commands (arbitrary codes) >>>>

        // Power unit modules commands
        SET_VOLTAGE_AND_CURRENT           = 0xEF01,
        //RESERVED                          = 0xEF02,
        //RESERVED                          = 0xEF03,
        GET_VOLTAGE_AND_CURRENT           = 0xEF04,

        GET_DEVICE_CLASS                  = 0xEF05,
        GET_NOMINAL_CURRENT               = 0xEF06,
        GET_NOMINAL_VOLTAGE               = 0xEF07,
        GET_NOMINAL_POWER                 = 0xEF08,
        GET_OVP_THRESHOLD                 = 0xEF09,
        GET_OCP_THRESHOLD                 = 0xEF0A,
        //RESERVED                          = 0xEF0B,
        SET_OVP_THRESHOLD                 = 0xEF0C,
        SET_OCP_THRESHOLD                 = 0xEF0D,
        SET_SET_VALUE_U                   = 0xEF0E,
        SET_SET_VALUE_I                   = 0xEF0F,
        PSC_SWITCH_POWER_OUTPUT_ON        = 0xEF10,
        PSC_SWITCH_POWER_OUTPUT_OFF       = 0xEF11,
        PSC_ACKNOWLEDGE_ALARMS            = 0xEF12,
        PSC_SWITCH_TO_REMOTE_CTRL         = 0xEF13,
        PSC_SWITCH_TO_MANUAL_CTRL         = 0xEF14,
        PSC_TRACKING_ON                   = 0xEF15,
        PSC_TRACKING_OFF                  = 0xEF16,

        // MKO module commands
        SEND_TEST_ARRAY                   = 0xFF05,
        RECEIVE_TEST_ARRAY                = 0xFF06,
        SEND_COMMAND_ARRAY                = 0xFF07,
        RECEIVE_COMMAND_ARRAY             = 0xFF08,
        SEND_TEST_ARRAY_FOR_CHANNEL       = 0xFF09,
        RECEIVE_TEST_ARRAY_FOR_CHANNEL    = 0xFF0A,
        SEND_COMMAND_ARRAY_FOR_CHANNEL    = 0xFF0B,
        RECEIVE_COMMAND_ARRAY_FOR_CHANNEL = 0xFF0C,
        SEND_TO_ANGLE_SENSOR              = 0xFF0D,

        // custom MKO comands
        START_MKO                         = 0xFF0E,
        STOP_MKO                          = 0xFF0F,

        // Logical commands
        GET_MODULE_STATUS                 = 0xFFF0, // returns two variables for physical and logical state, interaction with module 0 - impossible, 1 - possible
        SET_MODULE_LOGIC_STATUS           = 0xFFF1,  // set module logic state

        UNDEFINED                         = 0xFFFFFFFF
    };

    Q_ENUM(CommandID)

    enum ModuleAddress
    {
        CURRENT = 0x01,
        DEFAULT = 0x02,
    };

    enum ExecutionResult
    {
        CMD_OK      = 0x01,
        CMD_ERROR   = 0x02,
    };

    enum PowerState
    {
        POWER_ON,
        POWER_OFF
    };

    enum PowerSupplyChannelID
    {
        BUP_MAIN      = 1,
        BUP_RESERVE   = 2,
        UNKNOWN_3     = 3, // reserved
        HEATER_LINE_1 = 4,
        HEATER_LINE_2 = 5,
        DRIVE_CONTROL = 6,
    };

    Q_ENUM(PowerSupplyChannelID)

    enum MKOPowerSupplyChannelID
    {
        MKO_1 = 1,
        MKO_2 = 2,
        MKO_3 = 3,
        MKO_4 = 4
    };

    Q_ENUM(MKOPowerSupplyChannelID)

    enum ModuleID
    {
        POWER_UNIT_BUP,
        POWER_UNIT_PNA,
        MKO,
        STM,
        OTD,
        TECH,
        DRIVE_SIMULATOR,

        MODULES_COUNT
    };

    Q_ENUM(ModuleID)
};

#endif // MODULE_COMMANDS_H
