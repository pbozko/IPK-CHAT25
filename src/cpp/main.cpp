/**
 * Martin Bozko
 * xbozko01
 * 15.04.2025
 */

#include "../header/error.h"
#include "../header/arg_parser.h"
#include "../header/tcp_message.h"
#include "../header/tcp_client.h"
#include <string>
#include <iostream>
#include <csignal>

using namespace std;

FSMState FSM_STATE;
FSMState NEXT_STATE = START;

ClientTCP *client_tcp = nullptr;

void ctrlc(int signum){
    NEXT_STATE = END;
}

int main(int argc, char* argv[]){
    arg_parser parser(argc, argv);
    bool is_tcp = (parser.get_protocol() == "tcp");

    if(is_tcp){
        client_tcp = new ClientTCP(parser.get_server(), parser.get_port());
        client_tcp->connect_to_server();
    }

    signal(SIGINT, ctrlc);

    while(NEXT_STATE != END){
        FSM_STATE = NEXT_STATE;
        switch(FSM_STATE){
            case START:
                //cerr << "START state" << endl;
                if(is_tcp){
                    NEXT_STATE = client_tcp->start_state();
                }
                break;
            case AUTH:
                //cerr << "AUTH state" << endl;
                if(is_tcp){
                    NEXT_STATE = client_tcp->auth_state();
                }
                break;
            case OPEN:
                if(is_tcp){
                    NEXT_STATE = client_tcp->open_state();
                }
                break;
            case JOIN:
                if(is_tcp){
                    NEXT_STATE = client_tcp->join_state();
                }
                break;
            case ENDING:
                if(client_tcp){
                    client_tcp->send_bye();
                    client_tcp->close_connection();
                    delete client_tcp;
                }
                NEXT_STATE = END;
                break;
            default:
                break;
        }
    }
    return 0;
}