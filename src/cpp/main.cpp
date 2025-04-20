/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */

/**
 * Headers
 */
#include "../header/error.h"
#include "../header/arg_parser.h"
#include "../header/tcp_message.h"
#include "../header/tcp_client.h"
#include "../header/udp_message.h"
#include "../header/udp_client.h"
#include "../header/udp_message_values.h"
#include <string>
#include <iostream>
#include <csignal>

using namespace std;

// global variables for FSM state managing
FSMState FSM_STATE;
FSMState NEXT_STATE = START;

// global pointers for access in signal handler function
ClientTCP *client_tcp = nullptr;
ClientUDP *client_udp = nullptr;

// Ctrl+C interrupt signal handler
void ctrlc(int signum){
    // client is disconnecting, send BYE message to server
    if(client_tcp){
        client_tcp->send_bye();
    } else{
        client_udp->send_bye();
    }

    // close connection
    if(client_tcp){
        client_tcp->close_connection();
        delete client_tcp;
    }
    if(client_udp){
        client_udp->ending_state();
        client_udp->close_socket();
        delete client_udp;
    }
    exit(0);
}

int main(int argc, char* argv[]){
    signal(SIGINT, ctrlc);

    arg_parser parser(argc, argv);
    bool is_tcp = (parser.get_protocol() == "tcp");

    // connect based on chosen protocol
    if(is_tcp){
        client_tcp = new ClientTCP(parser.get_server(), parser.get_port());
        client_tcp->connect_to_server();
    } else{
        client_udp = new ClientUDP(parser.get_server(), parser.get_port(), parser.get_udp_timeout(), parser.get_udp_retransmission());
        client_udp->verify_address();
    }

    /**
     * Finite State Machine representing IPK25-CHAT client operation
     * Each state calls appropriate protocol function
     * Next state is decided based on the result of the current state
     */
    while(NEXT_STATE != END){
        FSM_STATE = NEXT_STATE;
        switch(FSM_STATE){
            case START:
                if(is_tcp){
                    NEXT_STATE = client_tcp->start_state();
                } else{
                    NEXT_STATE = client_udp->start_state();
                }
                break;
            case AUTH:
                if(is_tcp){
                    NEXT_STATE = client_tcp->auth_state();
                } else{
                    NEXT_STATE = client_udp->auth_state();
                }
                break;
            case OPEN:
                if(is_tcp){
                    NEXT_STATE = client_tcp->open_state();
                } else{
                    NEXT_STATE = client_udp->open_state();
                }
                break;
            case JOIN:
                if(is_tcp){
                    NEXT_STATE = client_tcp->join_state();
                } else{
                    NEXT_STATE = client_udp->join_state();
                }
                break;
            case BYE:
                if(client_tcp){
                    client_tcp->send_bye();
                } else{
                    client_udp->send_bye();
                }
                NEXT_STATE = ENDING;
                break;
            case ENDING:
                // terminate connection gracefully
                if(client_tcp){
                    client_tcp->close_connection();
                    delete client_tcp;
                }
                if(client_udp){
                    client_udp->ending_state();
                    client_udp->close_socket();
                    delete client_udp;
                }
                NEXT_STATE = END;
                break;
            default:
                break;
        }
    }
    return 0;
}