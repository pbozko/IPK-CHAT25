/**
 * Martin Bozko
 * xbozko01
 * 18.11.2024 for ISA project
 * 15.04.2025 updated for IPK project
 */
#ifndef ERROR_H
#define ERROR_H

#include <string>

using namespace std;

typedef enum CError {
    ARG_FEW = 1,
    ARG_MANY = 2,
    ARG_WRONG = 3,
    ARG_REQ = 4,
    PORT_VAL = 5,
    TIMEOUT_VAL = 6,
    RETR_VAL = 7,
    CONVERSION = 8,
    PROTOCOL = 9,
    MISSING_ATT = 10,
    INVALID_ATT = 11,

    SOCK_CREATE = 90,
    SOCK_SEND = 91,
    SOCK_RECV = 92,
} CError;

class error {
    public:
        string error_message;
        error(CError error_code);
};

#endif