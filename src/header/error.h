/**
 * Martin Bozko
 * xbozko01
 * 15.04.2025
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
    UINT16 = 8,
    L4_PROT = 9,
} CError;

class error {
    public:
        string error_message;
        error(CError error_code);
};

#endif