/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */
#ifndef UDP_MESSSAGE_H
#define UDP_MESSSAGE_H

#include <vector>
#include <cstdint>
#include <optional>
#include <regex>
#include "../header/udp_message_values.h"

using namespace std;

class MessageUDP {
    public:
        MessageUDP(const MSG_VAL type, const vector<uint8_t> &payload,
            const optional<uint16_t> id = nullopt,
            const optional<uint8_t> result = nullopt,
            const optional<uint16_t> ref_id = nullopt,
            const optional<string> &content = nullopt,
            const optional<string> &username = nullopt,
            const optional<string> &display_name = nullopt,
            const optional<string> &secret = nullopt,
            const optional<string> &channel = nullopt);

        class Builder;
        bool build();
        bool parse();
        void dump();
        MSG_VAL get_type();
        vector<uint8_t> get_payload();
        uint16_t get_id();
        uint8_t get_result();
        uint16_t get_ref_id();
        string get_content();
        string get_username();
        string get_display_name();
        string get_secret();
        string get_channel();
        string get_printable_payload();

    private:
        MSG_VAL type;
        vector<uint8_t> payload;
        optional<uint16_t> id;
        optional<uint8_t> result;
        optional<uint16_t> ref_id;
        optional<string> content;
        optional<string> username;
        optional<string> display_name;
        optional<string> secret;
        optional<string> channel;  
};

class MessageUDP::Builder{
    public:
        Builder(const MSG_VAL type, const vector<uint8_t> &payload);
        Builder& set_id(const uint16_t value);
        Builder& set_result(const uint8_t value);
        Builder& set_ref_id(const uint16_t value);
        Builder& set_content(const string &value);
        Builder& set_username(const string &value);
        Builder& set_display_name(const string &value);
        Builder& set_secret(const string &value);
        Builder& set_channel(const string &value);
        MessageUDP construct();

    private:
        MSG_VAL type;
        vector<uint8_t> payload;
        optional<uint16_t> id;
        optional<uint8_t> result;
        optional<uint16_t> ref_id;
        optional<string> content;
        optional<string> username;
        optional<string> display_name;
        optional<string> secret;
        optional<string> channel;  
};

#endif
