/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include "../header/udp_client.h"
#include "../header/error.h"
#include "../header/string_functions.h"
#include "../header/udp_message.h"

#define BUFFER_SIZE 4096

using namespace std;

ClientUDP::ClientUDP(const string &server, uint16_t port)
    : server(server), port(port), socket_i(), display_name("Unknown"), stream_buffer(""), fsm_state(START), awaiting_reply(false) {}

string ClientUDP::get_server(){
    return this->server;
}

uint16_t ClientUDP::get_port(){
    return this->port;
}

SocketUDP ClientUDP::get_socket_i(){
    return this->socket_i;
}

string ClientUDP::get_display_name(){
    return this->display_name;
}

void ClientUDP::set_display_name(const string &new_name){
    this->display_name = new_name;
}

void ClientUDP::verify_address(){
    this->socket_i.create();

    if(inet_addr(this->server.c_str()) == INADDR_NONE){
        in_addr ip_address = {};
        addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_DGRAM
        };

        addrinfo *resolved = nullptr;
        int result = getaddrinfo(this->server.c_str(), nullptr, &hints, &resolved);
        if(result != 0 || resolved == nullptr){
            throw fatal_error(SERV_RESL, "Failed to resolve hostname to IP address.");
        }

        ip_address = ((struct sockaddr_in*)resolved->ai_addr)->sin_addr;
        this->socket_i.set_connection(ip_address.s_addr, this->port);

        freeaddrinfo(resolved);
    } else{
        this->socket_i.set_connection(inet_addr(this->server.c_str()), this->port);
    }
}

void ClientUDP::close_socket(){
    if(this->socket_i.get_fd() > 0){
        this->socket_i.close();
    }
    this->fsm_state = ENDING;
}

