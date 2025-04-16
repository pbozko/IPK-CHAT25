/**
 * Martin Bozko
 * xbozko01
 * 16.04.2025
 */
#ifndef MESSAGE_VALUES_H
#define MESSAGE_VALUES_H

#include <cstdint>

typedef enum MSG_VAL : uint8_t {
    CONFIRM = 0x00,
    REPLY = 0x01,
    AUTH = 0x02,
    JOIN = 0x03,
    MSG = 0x04,
    PING = 0xFD,
    ERR = 0xFE,
    BYE = 0xFF
} MSG_VAL;

#endif