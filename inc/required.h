#ifndef REQUIRED_H
#define REQUIRED_H

// Команды управления УЦОС
typedef enum SIZE_OF_ENUM_UINT16
{
    UCOS_CMD_RESPONSE = (0x8000u),
    UCOS_CMD_BR = (0x0002u), // рабочий режим
    UCOS_CMD_RO = (0x0004u), // режим ожидания
    UCOS_CMD_FSET = (0x0005u), // FSET
    UCOS_CMD_TUNE = (0x0006u),
    UCOS_CMD_CURRENT_STATUS = (0x0009u), //(0x8009u)
    UCOS_CMD_RXDATA = (0x0011u), //(0x8011u)
    UCOS_CMD_SLEVEL = (0x0012u), //(0x8012u)
    UCOS_CMD_TXDATA = (0x0013u),
    UCOS_CMD_CHECK_A = (0x0021u),
    UCOS_CMD_RCHECK_R = (0x0022u),
    UCOS_CMD_RCHECK_T = (0x0023u),
    UCOS_CMD_CHECK_TEST = (0x0024u),
    UCOS_CMD_FCS_RANGE = (0x0061u),
    UCOS_CMD_SUM_CHECK = (0x0063u),
    UCOS_CMD_UUIO_SUMCHECK = (0x0064u), //(0x8064u)
    UCOS_CMD_CODE = (0x0065u),
    UCOS_CMD_UNKNOWN = (0x0071u), //(0x8071u)
} Modem_CMD_Names;

#endif // REQUIRED_H
