/**
 * Martin Bozko
 * xbozko01
 * 18.11.2024 for ISA project
 * 18.04.2025 updated for IPK project
 */
#ifndef TCP_SOCKET_INTERFACE_H
#define TCP_SOCKET_INTERFACE_H

#include <string>
#include <netinet/in.h>

using namespace std;

class SocketTCP{
    public:
        SocketTCP(const bool protocol);
        int get_fd();
        sockaddr_in get_connection();
        void set_connection(in_addr_t ip_address, uint16_t port);
        void create();
        void close();
        bool send(string data);
        string receive(int buffer_size);
    private:
        int fd;
        sockaddr_in connection;
};

#endif