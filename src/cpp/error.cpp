/**
 * Martin Bozko
 * xbozko01
 * 18.11.2024 for ISA project
 * 15.04.2025 modified for IPK project
 * 
 * Custom error definitions
 */

/**
 * Headers
 */
#include <string>
#include "../header/error.h"
#include <iostream>

using namespace std;

/**
 * Class for fatal errors - application exiting
 */
fatal_error::fatal_error(CError error_code, const string& details){
    string error_message;
    switch(error_code){
        case(ARG_ERR):
            error_message = "ARG_ERR: " + details;
            break;
        case(ARG_VAL):
            error_message = "ARG_VAL_ERR: " + details;
        case(CONV_ERR):
            error_message = "CONV_ERR: " + details;
            break;
        case(SOCK_CREATE):
            error_message = "SOCK_CREATE: " + details;
            break;
        case(SOCK_SEND):
            error_message = "SOCK_SEND: " + details;
            break;
        case(SOCK_RECV):
            error_message = "SOCK_RECV: " + details;
            break;
        case(SOCK_NONX):
            error_message = "SOCK_NONX: " + details;
            break;
        case(SOCK_CLOS):
            error_message = "SOCK_CLOS: " + details;
            break;
        case(SERV_RESL):
            error_message = "SERV_RESL: " + details;
            break;
        case(SERV_CONN):
            error_message = "SERV_CONN: " + details;
            break;
    }
    cout << error_message << endl;
    exit(error_code);
}

/**
 * Function for local errors - application doesn't exit
 */
void local_error(const string& error_message){
    cout << "ERROR: " << error_message << "\n";
}