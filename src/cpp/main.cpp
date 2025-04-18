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

// Client FSM states
#define START 1
#define AUTH 2
#define OPEN 3
#define JOIN 4
#define END 0

using namespace std;

int main(int argc, char* argv[]){
    arg_parser parser(argc, argv);
    bool is_tcp = (parser.get_protocol() == "tcp");

    ClientTCP* client_tcp;
    //ClientUDP* client_udp;

    if(is_tcp){
        client_tcp = new ClientTCP(parser.get_server(), parser.get_port());
    } //else{
    //  client_udp = new ClientUDP(parser.get_server(), parset.get_port()); 
    //}

    char FSM_STATE;
    char NEXT_STATE = START;
    while(NEXT_STATE != END){
        FSM_STATE = NEXT_STATE;
        switch(FSM_STATE){
            case START:
                if(is_tcp){
                    NEXT_STATE = client_tcp->authenticate();
                }
                break;
            case AUTH:
    
                break;
            case OPEN:
    
                break;
            case JOIN:
    
                break;
        }
    }
    return 0;
}




/**
 *     MessageTCP server_reply = MessageTCP("REPLY", "REPLY OK IS this is the reply message\r\n");
    server_reply.parse();
    server_reply.dump();
    cout << endl;

    MessageTCP server_err = MessageTCP("ERR", "ERR FROM pbozko IS this is the error message\r\n");
    server_err.parse();
    server_err.dump();
    cout << endl;

    MessageTCP server_auth = MessageTCP("AUTH", "AUTH xbozko01 AS pbozko USING Heslo123.\r\n");
    server_auth.parse();
    server_auth.dump();
    cout << endl;

    MessageTCP server_join = MessageTCP("JOIN", "JOIN default AS pbozko\r\n");
    server_join.parse();
    server_join.dump();
    cout << endl;

    MessageTCP server_msg = MessageTCP("MSG", "MSG FROM pbozko IS this is the msg content\r\n");
    server_join.parse();
    server_join.dump();
    cout << endl;

    MessageTCP server_bye = MessageTCP("BYE", "BYE FROM pbozko\r\n");
    server_bye.parse();
    server_bye.dump();
    cout << endl;
 */