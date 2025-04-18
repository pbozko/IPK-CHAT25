/**
 * Martin Bozko
 * xbozko01
 * 16.04.2025
 */

#include <string>
#include <optional>
#include <iostream>
#include "../header/message_values.h"
#include "../header/error.h"
#include "../header/tcp_message.h"
#include "../header/string_manipulation.h"

using namespace std;

MessageTCP::MessageTCP(const string &type, const string &payload,
                    const optional<string> &display_name,
                    const optional<string> &content,
                    const optional<string> &username,
                    const optional<string> &secret,
                    const optional<string> &channel,
                    const optional<string> &is_ok)
                : type(type), payload(payload), display_name(display_name), content(content),
                username(username), secret(secret), channel(channel), is_ok(is_ok) {}

void MessageTCP::build(){
    /**
     * TODO: check if passed params are valid
     */

    if(type == "ERR"){
        if(display_name && content){
            payload = "ERR FROM " + display_name.value() + " IS " + content.value();
        } //else throw error(MISSING_ATT);
        /**
         * TODO: what does missing attribute result in?
         */
    } else if(type == "REPLY"){
        if(content && is_ok){
            payload = "REPLY " + is_ok.value() + " IS " + content.value();
        }
    } else if(type == "AUTH"){
        if(username && display_name && secret){
            payload = "AUTH " + username.value() + " AS " + display_name.value() + " USING " + secret.value();
        }
    } else if(type == "JOIN"){
        if(channel && display_name){
            payload = "JOIN " + channel.value() + " AS " + display_name.value();
        } 
    } else if(type == "MSG"){
        if(display_name && content){
            payload = "MSG FROM " + display_name.value() + " IS " + content.value();
        }
    } else if(type == "BYE"){
        if(display_name){
            payload = "BYE FROM " + display_name.value();
        }
    } else{
        /**
         * TODO: what is default unexpected behavior?
         */
    }    
    payload += "\r\n";
}

void MessageTCP::parse(){
    if(type == "ERR"){
        display_name = extract_value(payload, "ERR FROM ", " IS ");
        content = extract_value(payload, " IS ", "\r\n");
        /**
         * TODO: malformed message
         */
    } else if(type == "REPLY"){
        is_ok = extract_value(payload, "REPLY ", " IS ");
        content = extract_value(payload, " IS ", "\r\n");
    } else if(type == "AUTH"){
        username = extract_value(payload, "AUTH ", " AS ");
        display_name = extract_value(payload, " AS ", " USING ");
        secret = extract_value(payload, " USING ", "\r\n");
    } else if(type == "JOIN"){
        channel = extract_value(payload, "JOIN ", " AS ");
        display_name = extract_value(payload, " AS ", "\r\n");
    } else if(type == "MSG"){
        display_name = extract_value(payload, " FROM ", " IS ");
        content = extract_value(payload, " IS ", "\r\n");
    } else if(type == "BYE"){
        display_name = extract_value(payload, " FROM ", "\r\n");
    } else{
        /**
         * TODO: what is default unexpected behavior?
         */
    }
}

void MessageTCP::dump(){
    cout << "type: " << type << endl;
    cout << "payload: " << payload;
    if(display_name)
        cout << "display_name: " << display_name.value() << endl;
    if(content)
        cout << "content: " << content.value() << endl;
    if(username)
        cout << "username: " << username.value() << endl;
    if(secret)
        cout << "secret: " << secret.value() << endl;
    if(channel)
        cout << "channel: " << channel.value() << endl;
    if(is_ok)
        cout << "is_ok: " << is_ok.value() << endl;
}

MessageTCP::Builder::Builder(const string &type, const string &payload)
    : type(type), payload(payload) {} /* , display_name(nullopt), content(nullopt),
    username(nullopt), secret(nullopt), channel(nullopt), is_ok(nullopt) */

MessageTCP::Builder& MessageTCP::Builder::set_display_name(const string &value){
    display_name = value;
    return *this;
}

MessageTCP::Builder& MessageTCP::Builder::set_content(const string &value){
    content = value;
    return *this;
}

MessageTCP::Builder& MessageTCP::Builder::set_username(const string &value){
    username = value;
    return *this;
}

MessageTCP::Builder& MessageTCP::Builder::set_secret(const string &value){
    secret = value;
    return *this;
}

MessageTCP::Builder& MessageTCP::Builder::set_channel(const string &value){
    channel = value;
    return *this;
}

MessageTCP::Builder& MessageTCP::Builder::set_is_ok(const string &value){
    is_ok = value;
    return *this;
}

MessageTCP MessageTCP::Builder::construct(){
    return MessageTCP(type, payload, display_name, content, username, secret, channel, is_ok);
}
