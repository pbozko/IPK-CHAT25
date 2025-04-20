/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */
#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include <string>
#include "../header/udp_socket_interface.h"
#include "../header/udp_message.h"
#include "../header/udp_message_values.h"
#include "../header/fsm_states.h"

using namespace std;

class ClientUDP{
    public:
        ClientUDP(const string &server, uint16_t port);
        void verify_address();
        void close_socket();

        string get_server();
        uint16_t get_port();
        SocketUDP get_socket_i();
        void set_display_name(const string &new_name);
        string get_display_name();

    private:
        string server;
        uint16_t port;
        SocketUDP socket_i;
        string display_name;
        string stream_buffer;
        FSMState fsm_state;
        vector<string> input_buffer;
        bool awaiting_reply;
};

#endif