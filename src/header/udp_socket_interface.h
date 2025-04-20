/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */
#ifndef UDP_SOCKET_INTERFACE_H
#define UDP_SOCKET_INTERFACE_H

#include <string>
#include <vector>
#include <netinet/in.h>

using namespace std;

class SocketUDP {
    public:
        SocketUDP();
        int get_fd();
        sockaddr_in get_connection();
        void set_connection(in_addr_t ip_address, uint16_t port);
        void update_port(uint16_t port);
        void create();
        void close();
        bool send(const vector<uint8_t>& data);
        vector<uint8_t> receive(int buffer_size);

    private:
        int fd;
        sockaddr_in connection;
};

#endif