/**
 * Martin Bozko
 * xbozko01
 * 16.04.2025
 */

#include <string>
#include "../header/tcp_socket_interface.h"

using namespace std;

class ClientTCP{
    public:
        ClientTCP(const string &server, uint16_t port);
        bool connect_to_server();

    private:
        string server;
        uint16_t port;
        SocketTCP socket_i;

};