/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */
#ifndef MESSAGE_VALUES_H
#define MESSAGE_VALUES_H

#include <cstdint>

typedef enum MSG_VAL : uint8_t{
    CONFIRM_MSG = 0x00,
    REPLY_MSG = 0x01,
    AUTH_MSG = 0x02,
    JOIN_MSG = 0x03,
    MSG_MSG = 0x04,
    PING_MSG = 0xFD,
    ERR_MSG = 0xFE,
    BYE_MSG = 0xFF,
    MALFORMED_MSG = 0x10,
    UNKNOWN_MSG = 0x11,
    ALREADY_PROCESSED = 0x20,
} MSG_VAL;

#endif