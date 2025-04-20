/**
 * Martin Bozko
 * xbozko01
 * 16.04.2025
 */

#include <string>
#include <optional>
#include <iostream>
#include "../header/error.h"
#include "../header/tcp_message.h"
#include "../header/string_functions.h"

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

string MessageTCP::get_payload(){
    return this->payload;
}

string MessageTCP::get_type(){
    return this->type;
}

string MessageTCP::get_display_name(){
    return this->display_name.value();
}

string MessageTCP::get_content(){
    return this->content.value();
}

string MessageTCP::get_username(){
    return this->username.value();
}

string MessageTCP::get_secret(){
    return this->secret.value();
}

string MessageTCP::get_channel(){
    return this->channel.value();
}

string MessageTCP::get_is_ok(){
    return this->is_ok.value();
}

bool MessageTCP::build(){
    if(type == "ERR"){
        if(display_name && check_dname(display_name.value()) && content && check_content(content.value())){
            payload = "ERR FROM " + display_name.value() + " IS " + content.value();
        } else return false;
    } /*else if(type == "REPLY"){
        if(content && check_content(content.value()) && is_ok && check_ok(is_ok.value())){
            payload = "REPLY " + is_ok.value() + " IS " + content.value();                      client reply message doesn't exist
        } else return false;
    }*/ else if(type == "AUTH"){
        if(username && check_id(username.value()) && display_name && check_dname(display_name.value()) && secret && check_secret(secret.value())){
            payload = "AUTH " + username.value() + " AS " + display_name.value() + " USING " + secret.value();
        } else return false;
    } else if(type == "JOIN"){
        if(channel && check_id(channel.value()) && display_name && check_dname(display_name.value())){
            payload = "JOIN " + channel.value() + " AS " + display_name.value();
        }else return false;
    } else if(type == "MSG"){
        if(display_name && check_dname(display_name.value()) && content && check_content(content.value())){
            payload = "MSG FROM " + display_name.value() + " IS " + content.value();
        } else return false;
    } else if(type == "BYE"){
        if(display_name && check_dname(display_name.value())){
            payload = "BYE FROM " + display_name.value();
        } else return false;
    } else{
        return false; // malformed message
    }    
    payload += "\r\n";
    return true;
}

bool MessageTCP::parse(){
    if(type == "ERR"){
        display_name = extract_value(payload, "ERR FROM ", " IS ");
        content = extract_value(payload, " IS ", "\r\n");
        if(!(check_dname(display_name.value()) && check_content(content.value())))
            return false;
    } else if(type == "REPLY"){
        is_ok = extract_value(payload, "REPLY ", " IS ");
        content = extract_value(payload, " IS ", "\r\n");
        if(!check_content(content.value()))
            return false;
    } else if(type == "MSG"){
        display_name = extract_value(payload, " FROM ", " IS ");
        content = extract_value(payload, " IS ", "\r\n");
        if(!(check_dname(display_name.value()) && check_content(content.value())))
            return false;
    } else if(type == "BYE"){
        display_name = extract_value(payload, " FROM ", "\r\n");
        if(!check_dname(display_name.value()))
            return false;
    } else{
        return false; // malformed message
    }
    return true;
}

MessageTCP::Builder::Builder(const string &type, const string &payload)
    : type(type), payload(payload) {}

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

/**
 * DEBUG FUNCTION
 */
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