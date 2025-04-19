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

typedef enum CError{
    ARG_ERR = 1,
    ARG_VAL = 2,
    CONV_ERR = 3,
    SOCK_CREATE = 51,
    SOCK_SEND = 52,
    SOCK_RECV = 53,
    SOCK_NONX = 54,
    SOCK_CLOS = 55,
    SERV_RESL = 56,
    SERV_CONN = 57,
} CError;

class fatal_error{
    public:
        fatal_error(CError error_code, const string& details);
    private:
        string details;
};

void local_error(const string& error_message);

#endif