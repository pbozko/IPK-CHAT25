/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */
#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <string>
#include <chrono>
#include "../header/tcp_socket_interface.h"
#include "../header/tcp_message.h"
#include "../header/fsm_states.h"

using namespace std;

class ClientTCP{
    public:
        ClientTCP(const string &server, uint16_t port);
        void connect_to_server();
        void close_connection();

        MessageTCP process_message();
        void send_auth(const string &username, const string &secret, const string &display_name);
        void send_join(const string &channel);
        void send_bye();
        void send_err(const string& error_message);
        void send_msg(const string& text_content);
        FSMState error_to_server(const string& error_message);
        bool check_reply();
        bool parse_as_command(const vector<string> &input);

        FSMState send_message(bool read_flag, const string &buffer_input);

        FSMState read_stream();
        FSMState empty_input_buffer();

        FSMState state_process(FSMState NEW_FSM_STATE);

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
        chrono::steady_clock::time_point reply_expect_begin;
};

#endif