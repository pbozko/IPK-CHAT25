/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */
#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <string>
#include <cstdint>

using namespace std;

class arg_parser {
    public:
        arg_parser(int argc, char** argv);
        string get_protocol();
        string get_server();
        uint16_t get_port();
        uint16_t get_udp_timeout();
        uint16_t get_udp_retransmission();
        bool get_output_help_flag();
        void print_info();

    private:
        int argc;
        char** argv;
        string protocol;
        bool t_flag; // -t is set
        string server;
        bool s_flag; // -s is set
        uint16_t port;
        uint16_t udp_timeout;
        uint8_t udp_retransmission;
        bool output_help_flag;
        void parse_input();
};

#endif