/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */
#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include <string>
#include <unordered_set>
#include <chrono>
#include "../header/udp_socket_interface.h"
#include "../header/udp_message.h"
#include "../header/udp_message_values.h"
#include "../header/fsm_states.h"

using namespace std;

class ClientUDP{
    public:
        ClientUDP(const string &server, uint16_t port, uint16_t timeout, uint8_t max_retries);
        void verify_address();
        void close_socket();

        MessageUDP process_message(const vector<uint8_t> payload);
        void send_auth(const string &username, const string &secret, const string &display_name);
        void send_join(const string &channel);
        void send_bye();
        void send_err(const string& error_message);
        void send_msg(const string& text_content);
        void send_confirm(const uint16_t ref_id);
        FSMState error_to_server(const string& error_message);
        bool check_reply();
        bool parse_as_command(const vector<string> &input);

        FSMState send_message(bool read_flag, const string &buffer_input);

        FSMState send_in_auth(const string& input);
        FSMState send_in_open(const string& input);

        FSMState read_datagram();
        FSMState empty_input_buffer();
        bool retransmit_if_timeout();

        FSMState state_process(FSMState NEW_FSM_STATE);

        FSMState start_state();
        FSMState auth_state();
        FSMState open_state();
        FSMState join_state();
        void ending_state();

        string get_server();
        uint16_t get_port();
        SocketUDP get_socket_i();
        uint16_t get_timeout();
        uint8_t get_max_retries();
        void set_display_name(const string &new_name);
        string get_display_name();

    private:
        string server;
        uint16_t port;
        SocketUDP socket_i;
        uint16_t timeout;
        uint8_t max_retries;

        uint16_t message_id;
        string display_name;
        string stream_buffer;
        FSMState fsm_state;
        vector<string> input_buffer;
        bool awaiting_reply;
        chrono::steady_clock::time_point reply_expect_begin;
        bool awaiting_confirm;

        MessageUDP last_message;
        unordered_set<uint16_t> processed_ids;
        pair<uint16_t, chrono::steady_clock::time_point> unconfirmed_message;
        uint8_t retry_count;

        bool require_confirm;
        bool require_time;
};

#endif