/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */

#include <string>
#include <optional>
#include <iostream>
#include <arpa/inet.h>
#include "../header/error.h"
#include "../header/udp_message_values.h"
#include "../header/udp_message.h"
#include "../header/string_functions.h"

using namespace std;

MessageUDP::MessageUDP(const MSG_VAL type, const vector<uint8_t> &payload,
                    const optional<uint16_t> id,
                    const optional<uint8_t> result,
                    const optional<uint16_t> ref_id,
                    const optional<string> &content,
                    const optional<string> &username,
                    const optional<string> &display_name,
                    const optional<string> &secret,
                    const optional<string> &channel)
                : type(type), payload(payload), id(id), result(result), ref_id(ref_id), content(content), 
                username(username), display_name(display_name), secret(secret), channel(channel) {}

vector<uint8_t> MessageUDP::get_payload(){
    return this->payload;
}

uint16_t MessageUDP::get_id(){
    return this->id.value();
}

uint8_t MessageUDP::get_result(){
    return this->result.value();
}

uint16_t MessageUDP::get_ref_id(){
    return this->ref_id.value();
}

string MessageUDP::get_content(){
    return this->content.value();
}

string MessageUDP::get_username(){
    return this->username.value();
}

string MessageUDP::get_display_name(){
    return this->display_name.value();
}

string MessageUDP::get_secret(){
    return this->secret.value();
}

string MessageUDP::get_channel(){
    return this->channel.value();
}

void push_two_bytes(vector<uint8_t> &payload, uint16_t value){
    value = htons(value);
    payload.push_back((value >> 8) & 0xFF); // high byte
    payload.push_back(value & 0xFF);        // low byte
}

uint16_t pop_two_bytes(const vector<uint8_t> &payload, size_t index){
    uint16_t value = (payload[index] << 8 | payload[index + 1]);
    return ntohs(value);
}

void push_string(vector<uint8_t> &payload, const vector<string> &values){
    for(string value : values){
        payload.insert(payload.end(), value.begin(), value.end());
        payload.push_back('\0');
    }
}

vector<string> pop_string(vector<uint8_t> &payload, size_t n, size_t index){
    vector<string> values;

    for(size_t i = 0; i < n; i++){
        string value;

        while(index < payload.size() && payload[index] != '\0'){
            value += static_cast<char>(payload[index]);
            index++;
        }
        values.push_back(value);

        if(index < payload.size() && payload[index] == '\0'){
            index++;
        } else{
            /**
             * TODO: malformed message;
             */
            return {};
        }
    }

    return values;
}

bool MessageUDP::build(){
    this->payload.push_back(this->type);
    switch(this->type){
        case CONFIRM_MSG:
            if(this->ref_id){
                push_two_bytes(this->payload, this->ref_id.value());
            } else return false;
            break;
        case AUTH_MSG:
            if(this->id && this->username && check_id(this->username.value()) && this->display_name && check_dname(this->display_name.value()) && this->secret && check_secret(this->secret.value())){
                push_two_bytes(this->payload, this->id.value());
                push_string(this->payload, {this->username.value(), this->secret.value(), this->display_name.value()});
            } else return false;
            break;
        case JOIN_MSG:
            if(this->id && this->channel && check_id(this->channel.value()) && this->display_name && check_dname(this->display_name.value())){
                push_two_bytes(this->payload, this->id.value());
                push_string(this->payload, {this->channel.value(), this->display_name.value()});
            } else return false;
            break;
        case MSG_MSG:
        case ERR_MSG:
            if(this->id && this->display_name && check_dname(this->display_name.value()) && this->content && check_content(this->content.value())){
                push_two_bytes(this->payload, this->id.value());
                push_string(this->payload, {this->display_name.value(), this->content.value()});
            } else return false;
            break;
        case BYE_MSG:
            if(this->id && this->display_name && check_dname(this->display_name.value())){
                push_two_bytes(this->payload, this->id.value());
                push_string(this->payload, {this->display_name.value()});
            } else return false;
            break;
        default:
            break;
    }
    return true;
}

bool MessageUDP::parse(){
    /**
     * TODO: return false on malformed message - not conforming to grammar
     */
    size_t offset = 1;
    vector<string> string_values;

    this->type = static_cast<MSG_VAL>(this->payload[0]);

    switch(this->type){
        case CONFIRM_MSG:
            this->ref_id = pop_two_bytes(this->payload, offset);
            break;
        default:
            this->id = pop_two_bytes(this->payload, offset);
            break;
    }
    offset += 2;

    switch(this->type){
        case REPLY_MSG:
            this->result = this->payload[offset];
            offset++;
            this->ref_id = pop_two_bytes(this->payload, offset);
            offset += 2;
            string_values = pop_string(this->payload, 1, offset);
            this->content = string_values[0];
            break;
        case AUTH_MSG:
            string_values = pop_string(this->payload, 3, offset);
            this->username = string_values[0];
            this->secret = string_values[1];
            this->display_name = string_values[2];
            break;
        case JOIN_MSG:
            string_values = pop_string(this->payload, 2, offset);
            this->channel = string_values[0];
            this->display_name = string_values[1];
            break;
        case MSG_MSG:
        case ERR_MSG:
            string_values = pop_string(this->payload, 2, offset);
            this->display_name = string_values[0];
            this->content = string_values[1];
            break;
        case BYE_MSG:
            string_values = pop_string(this->payload, 1, offset);
            this->display_name = string_values[0];
            break;
        default:
            break;
    }
    return true;
}

void MessageUDP::dump(){
    cout << "type: " << type << endl;
    if(display_name)
        cout << "id: " << id.value() << endl;
    if(display_name)
        cout << "result: " << result.value() << endl;
    if(display_name)
        cout << "ref_id: " << ref_id.value() << endl;
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
}

MessageUDP::Builder::Builder(const MSG_VAL type, const vector<uint8_t> &payload)
    : type(type), payload(payload) {}

MessageUDP::Builder& MessageUDP::Builder::set_id(const uint16_t value){
    id = value;
    return *this;
}

MessageUDP::Builder& MessageUDP::Builder::set_result(const uint8_t value){
    result = value;
    return *this;
}

MessageUDP::Builder& MessageUDP::Builder::set_ref_id(const uint16_t value){
    ref_id = value;
    return *this;
}

MessageUDP::Builder& MessageUDP::Builder::set_content(const string &value){
    content = value;
    return *this;
}

MessageUDP::Builder& MessageUDP::Builder::set_username(const string &value){
    username = value;
    return *this;
}

MessageUDP::Builder& MessageUDP::Builder::set_display_name(const string &value){
    display_name = value;
    return *this;
}

MessageUDP::Builder& MessageUDP::Builder::set_secret(const string &value){
    secret = value;
    return *this;
}

MessageUDP::Builder& MessageUDP::Builder::set_channel(const string &value){
    channel = value;
    return *this;
}

MessageUDP MessageUDP::Builder::construct(){
    return MessageUDP(type, payload, id, result, ref_id, content, username, display_name, secret, channel);
}