/**
 * Martin Bozko
 * xbozko01
 * 16.04.2025
 */

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <string>
#include <string>
#include "../header/tcp_socket_interface.h"
#include "../header/tcp_message.h"

using namespace std;

typedef enum FSMState{
    START = 1,
    AUTH = 2,
    OPEN = 3,
    JOIN = 4,
    ENDING = 5,
    END = 0,
} FSMState;

class ClientTCP{
    public:
        ClientTCP(const string &server, uint16_t port);
        void connect_to_server();
        void close_connection();
        void interrupt(int signum);
        void send_bye();
        void send_err(const string& error_message);
        bool send_msg(const string& text_content);
        FSMState send_in_auth(const string& input);
        FSMState send_in_open(const string& input);
        FSMState read_stream();
        FSMState start_state();
        FSMState auth_state();
        FSMState open_state();
        FSMState join_state();
        MessageTCP process_message();
        FSMState empty_input_buffer();

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
        string stream_buffer;
        FSMState fsm_state;
        vector<string> input_buffer;
        bool awaiting_reply;
};

#endif