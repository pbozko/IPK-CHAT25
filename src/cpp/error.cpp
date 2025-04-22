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
 * Function for fatal errors - application exiting
 */
void fatal_error(CError error_code, const string& error_message){
    cout << error_message << endl;
    exit(error_code);
}

/**
 * Function for local errors - application doesn't exit
 */
void local_error(const string& error_message){
    cout << "ERROR: " << error_message << "\n";
}