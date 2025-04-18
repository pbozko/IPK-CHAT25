/**
 * Martin Bozko
 * xbozko01
 * 18.11.2024 for ISA project
 * 15.04.2025 modified for IPK project
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
        case(CONVERSION):
            this->error_message = "Error occured during type conversion.";
            break;
        case(PROTOCOL):
            this->error_message = "Specified L4 protocol is invalid. Options are either 'tcp' or 'udp'.";
            break;
        case(MISSING_ATT):
            this->error_message = "Missing required message attribute.";
            break;
    }

    cerr << this->error_message << endl;
    cerr << "Help message TODO" << endl;
    exit(error_code);
}