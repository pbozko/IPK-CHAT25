/**
 * Martin Bozko
 * xbozko01
 * 16.04.2025
 */

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <string>
#include "../header/tcp_socket_interface.h"

using namespace std;

class ClientTCP{
    public:
        ClientTCP(const string &server, uint16_t port);
        void connect_to_server();
        int authenticate();

        string get_server();
        uint16_t get_port();
        SocketTCP get_socket_i();
        void set_display_name(const string &new_name);
        string get_display_name();

    private:
        string server;
        uint16_t port;
        SocketTCP socket_i;
        string display_name;
};

#endif