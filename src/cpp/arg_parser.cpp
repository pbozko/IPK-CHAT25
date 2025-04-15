/**
 * Martin Bozko
 * xbozko01
 * 15.04.2025
 * 
 * Argument parser class implementation
 */

#include <getopt.h>
#include <iostream>
#include <algorithm>
#include <cctype>
#include "../header/arg_parser.h"
#include "../header/error.h"

using namespace std;

arg_parser::arg_parser(int argc, char** argv){
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

    this->parse_input();

    if(!(this->t_flag && this->s_flag)){
        throw error(ARG_REQ);
    }

    transform(this->protocol.begin(), this->protocol.end(), this->protocol.begin(), ::tolower);
    if(this->protocol != "tcp" && this->protocol != "udp"){
        throw error(L4_PROT);
    }
}

void arg_parser::parse_input(){
    int min_argc = 5; // executable, protocol + value, server + value
    int max_argc = 12;
    if(argc < min_argc){
        throw error(ARG_FEW);
    } else if(argc > max_argc){
        throw error(ARG_MANY);
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
                } catch(const std::invalid_argument& e){
                    throw error(PORT_VAL);
                } catch(const std::out_of_range& e){
                    throw error(UINT16);
                } break;
            case 'd':
                try{
                    this->udp_timeout = static_cast<uint16_t>(stoi(optarg));
                } catch(const std::invalid_argument& e){
                    throw error(TIMEOUT_VAL);
                } catch(const std::out_of_range& e){
                    throw error(UINT16);
                } break;
            case 'r':
                try{
                    this->udp_retransmission = static_cast<uint16_t>(stoi(optarg));
                } catch(const std::invalid_argument& e){
                    throw error(TIMEOUT_VAL);
                } catch(const std::out_of_range& e){
                    throw error(UINT16);
                } break;
            case 'h':
                this->output_help_flag = true;
                break;
            default:
                throw error(ARG_WRONG);
                break;
        }
    }
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
 