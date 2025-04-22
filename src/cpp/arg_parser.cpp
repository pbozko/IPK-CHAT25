/**
 * Martin Bozko
 * xbozko01
 * 18.11.2024 for ISA project
 * 15.04.2025 updated for IPK project
 * 
 * Argument parser class implementation
 */

/**
 * Headers
 */
#include <getopt.h>
#include <iostream>
#include <algorithm>
#include <cctype>
#include "../header/arg_parser.h"
#include "../header/error.h"

using namespace std;

/**
 * Constructor for argument parser class.
 */
arg_parser::arg_parser(int argc, char** argv){
    // set default/initial values
    this->argc = argc;
    this->argv = argv;
    this->protocol = "";
    this->t_flag = false;
    this->server = "";
    this->s_flag = false;
    this->port = 4567;
    this->udp_timeout = 250;
    this->udp_retransmission = 3;
    this->output_help_flag = false;

    // parse arguments
    this->parse_input();

    if(this->output_help_flag){
        cout    << "Available commands: " << endl
                << "/auth <username> <secret> <display_name>    - log in to chat server." << endl
                << "/join <channel_id>                          - change chat channel." << endl
                << "/rename <display_name>                      - changes session display name." << endl
                << "/help                                       - print this help message." << endl
                << "Ctrl + C                                    - send BYE to server and exit." << endl
                << "Ctrl + D                                    - send BYE to server and exit." << endl;
        exit(0);
    }

    // check for necessary arguments
    if(!(this->t_flag && this->s_flag)){
        fatal_error(ARG_ERR, "Missing one or more required parameters (-t <protocol> -s <ip_addr/hostname>).");
    }

    // convert protocol name string to lowercase
    transform(this->protocol.begin(), this->protocol.end(), this->protocol.begin(), ::tolower);
    // check for valid protocol option
    if(this->protocol != "tcp" && this->protocol != "udp"){
        fatal_error(ARG_VAL, "Protocol can only be 'tcp' or 'udp'.");
    }
}

/**
 * Parses arguments into class attributes
 * @return void
 */
void arg_parser::parse_input(){
    int min_argc = 5; // executable, protocol + value, server + value
    int max_argc = 12;
    if(argc < min_argc){
        fatal_error(ARG_ERR, "Too few arguments.");
    } else if(argc > max_argc){
        fatal_error(ARG_ERR, "Too many arguments.");
    }

    int arg = 0;
    while((arg = getopt(this->argc, this->argv, "t:s:p:d:r:h")) != -1){
        switch(arg){
            case 't':
                this->protocol = (string)optarg;
                this->t_flag = true;
                break;
            case 's':
                this->server = (string)optarg;
                this->s_flag = true;
                break;
            case 'p':
                try{
                    this->port = static_cast<uint16_t>(stoi(optarg));
                } catch(const invalid_argument& e){
                    fatal_error(ARG_VAL, "Port number must be an integer.");
                } catch(const out_of_range& e){
                    fatal_error(CONV_ERR, "Error converting specified port number to uint16_t.");
                } break;
            case 'd':
                try{
                    this->udp_timeout = static_cast<uint16_t>(stoi(optarg));
                } catch(const invalid_argument& e){
                    fatal_error(ARG_VAL, "UDP timeout value must be an integer.");
                } catch(const out_of_range& e){
                    fatal_error(ARG_VAL, "Error converting UDP timeout value to uint16_t.");
                } break;
            case 'r':
                try{
                    this->udp_retransmission = static_cast<uint16_t>(stoi(optarg));
                } catch(const invalid_argument& e){
                    fatal_error(ARG_VAL, "UDP retransmission value must be an integer.");
                } catch(const out_of_range& e){
                    fatal_error(ARG_VAL, "Error converting UDP retransmission value to uint16_t.");
                } break;
            case 'h':
                this->output_help_flag = true;
                break;
            default:
                fatal_error(ARG_ERR, "Unrecognized argument.");
                break;
        }
    }
}

/**
 * Getter functions
 */
string arg_parser::get_protocol(){
    return this->protocol;
}

string arg_parser::get_server(){
    return this->server;
}

uint16_t arg_parser::get_port(){
    return this->port;
}

uint16_t arg_parser::get_udp_timeout(){
    return this->udp_timeout;
}

uint16_t arg_parser::get_udp_retransmission(){
    return this->udp_retransmission;
}

bool arg_parser::get_output_help_flag(){
    return this->output_help_flag;
}


/**
 * DEBUG FUNCTION
 */
void arg_parser::print_info(){
    cout << "argc: " << this->argc << endl;
    cout << "protocol: " << this->protocol << endl;
    cout << "t_flag: " << this->t_flag << endl;
    cout << "server: " << this->server << endl;
    cout << "s_flag: " << this->s_flag << endl;
    cout << "port: " << this->port << endl;
    cout << "udp_timeout: " << this->udp_timeout << endl;
    cout << "udp_retransmission: " << this->udp_retransmission << endl;
    cout << "output_help_flag: " << this->output_help_flag << endl;
}
