/**
 * Martin Bozko
 * xbozko01
 * 16.04.2025
 */

#include <arpa/inet.h>
#include <netdb.h>
#include "../header/tcp_client.h"
#include "../header/error.h"
 
using namespace std;

ClientTCP::ClientTCP(const string &server, uint16_t port)
    : server(server), port(port), socket_i({}) {}

bool ClientTCP::connect_to_server(){
    this->socket_i.create();

    if(inet_addr(this->server.c_str()) == INADDR_NONE){
        in_addr ip_address = {};
        addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM
        };

        addrinfo *resolved = nullptr;
        int result = getaddrinfo(this->server.c_str(), nullptr, &hints, &resolved);
        if(result != 0 || resolved == nullptr) 
            //throw error(HOST_RES);
        
        ip_address = ((struct sockaddr_in*)resolved->ai_addr)->sin_addr;
        this->socket_i.set_connection(ip_address.s_addr, this->port);

        freeaddrinfo(resolved);
    } else{
        this->socket_i.set_connection(inet_addr(this->server.c_str()), this->port);
    }

    sockaddr_in connection = this->socket_i.get_connection();
    if(connect(this->socket_i.get_fd(), (struct sockaddr*)&connection, sizeof(connection)) == -1)
        ;// throw error(SERVER_CONN);
}

