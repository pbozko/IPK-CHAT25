/**
 * Martin Bozko
 * xbozko01
 * 15.04.2025
 * 
 * Custom error definitions
 */
#include <string>
#include "../header/error.h"
#include <iostream>

using namespace std;

error::error(CError error_code){
    switch(error_code){
        case(ARG_FEW):
            this->error_message = "Too few arguments.";
            break;
        case(ARG_MANY):
            this->error_message = "Too many arguments.";
            break;
        case(ARG_WRONG):
            this->error_message = "Unrecognized arguments.";
            break;
        case(ARG_REQ):
            this->error_message = "Missing required arguments.";
            break;
        case(PORT_VAL):
            this->error_message = "Specified port needs to be a number.";
            break;
        case(TIMEOUT_VAL):
            this->error_message = "Specified UDP confirmation timeout needs to be a number.";
            break;
        case(RETR_VAL):
            this->error_message = "Specified maximum UDP retransmission count needs to be a number.";
            break;
        case(UINT16):
            this->error_message = "Value cannot be converted to uint16_t.";
            break;
        case(L4_PROT):
            this->error_message = "Specified L4 protocol is invalid. Options are either 'tcp' or 'udp'.";
            break;
    }

    cerr << this->error_message << endl;
    cerr << "Help message TODO" << endl;
    exit(error_code);
}