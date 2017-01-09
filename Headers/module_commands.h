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
        GET_VOLTAGE_AND_CURRENT         = 0xFF04,

        // MKO module commands (0xFF0000 started)
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

    enum ModuleID
    {
        POWER_UNIT_BUP,
        POWER_UNIT_PNA,
        MKO,
        STM,
        OTD,
        TECH,

        MODULES_COUNT
    };
};

#endif // MODULE_COMMANDS_H
