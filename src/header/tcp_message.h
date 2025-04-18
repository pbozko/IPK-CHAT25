/**
 * Martin Bozko
 * xbozko01
 * 16.04.2025
 */

#ifndef MESSAGE_UDP_H
#define MESSAGE_UDP_H

#include <string>
#include <optional>
#include <regex>
#include "message_values.h"
#include "error.h"

using namespace std;

class MessageTCP {
    public:
        MessageTCP(const string &type, const string &payload,
            const optional<string> &display_name = nullopt,
            const optional<string> &content = nullopt,
            const optional<string> &username = nullopt, 
            const optional<string> &secret = nullopt,
            const optional<string> &channel = nullopt,
            const optional<string> &is_ok = nullopt);

        class Builder;
        void build();
        void parse();
        void dump();

    private:
        string type;
        string payload;
        optional<string> display_name;
        optional<string> content;
        optional<string> username;
        optional<string> secret;
        optional<string> channel;
        optional<string> is_ok;
};

class MessageTCP::Builder{
    public:
        Builder(const string& type, const string &payload);
        Builder& set_display_name(const string &value);
        Builder& set_content(const string &value);
        Builder& set_username(const string &value);
        Builder& set_secret(const string &value);
        Builder& set_channel(const string &value);
        Builder& set_is_ok(const string &value);
        MessageTCP construct();

    private:
        string type;
        string payload;
        optional<string> display_name;
        optional<string> content;
        optional<string> username;
        optional<string> secret;
        optional<string> channel;
        optional<string> is_ok;
};

#endif