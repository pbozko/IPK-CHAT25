/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include "../header/udp_client.h"
#include "../header/error.h"
#include "../header/string_functions.h"
#include "../header/udp_message.h"

#define BUFFER_SIZE 4096

using namespace std;

ClientUDP::ClientUDP(const string &server, uint16_t port, uint16_t timeout, uint8_t max_retries)
    : server(server), port(port), socket_i(), timeout(timeout), max_retries(max_retries), message_id(0),
    display_name("Unknown"), /*stream_buffer(""),*/ fsm_state(START), awaiting_reply(false), awaiting_confirm(false), last_message(UNKNOWN_MSG, {}), retry_count(0) {}

string ClientUDP::get_server(){
    return this->server;
}

uint16_t ClientUDP::get_port(){
    return this->port;
}

SocketUDP ClientUDP::get_socket_i(){
    return this->socket_i;
}

string ClientUDP::get_display_name(){
    return this->display_name;
}

void ClientUDP::set_display_name(const string &new_name){
    this->display_name = new_name;
}

uint16_t ClientUDP::get_timeout(){
    return this->timeout;
}
uint8_t ClientUDP::get_max_retries(){
    return this->max_retries;
}

vector<pollfd> get_file_descriptors(SocketUDP socket){
    vector<pollfd> fds(2);

    fds[0].fd = socket.get_fd();
    fds[0].events = POLLIN;

    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    return fds;
}

void ClientUDP::verify_address(){
    this->socket_i.create();

    if(inet_addr(this->server.c_str()) == INADDR_NONE){
        in_addr ip_address = {};
        addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_DGRAM
        };

        addrinfo *resolved = nullptr;
        int result = getaddrinfo(this->server.c_str(), nullptr, &hints, &resolved);
        if(result != 0 || resolved == nullptr){
            throw fatal_error(SERV_RESL, "Failed to resolve hostname to IP address.");
        }

        ip_address = ((struct sockaddr_in*)resolved->ai_addr)->sin_addr;
        this->socket_i.set_connection(ip_address.s_addr, this->port);

        freeaddrinfo(resolved);
    } else{
        this->socket_i.set_connection(inet_addr(this->server.c_str()), this->port);
    }
}

void ClientUDP::close_socket(){
    if(this->socket_i.get_fd() > 0){
        this->socket_i.close();
    }
    this->fsm_state = ENDING;
}

MessageUDP ClientUDP::process_message(const vector<uint8_t> payload){
    if(!payload.empty()){
        MessageUDP new_message(static_cast<MSG_VAL>(payload[0]), payload);
        if(new_message.parse()){
            if(new_message.get_type() != CONFIRM_MSG){
                if(this->processed_ids.find(new_message.get_id()) != this->processed_ids.end()){
                    return MessageUDP(ALREADY_PROCESSED, {});
                } else{
                    processed_ids.insert(new_message.get_id());
                }
            }
            switch(new_message.get_type()){
                case REPLY_MSG:{
                    string result = (new_message.get_result() == 1) ? "Success" : "Failure";
                    cout << "Action " << result << ": " << new_message.get_content() << "\n";
                    break;}
                case ERR_MSG:
                    cout << "ERROR FROM " << new_message.get_display_name() << ": " << new_message.get_content() << "\n";
                    break;
                case MSG_MSG:
                    cout << new_message.get_display_name() << ": " << new_message.get_content() << "\n";
                    break;
                case CONFIRM_MSG:{
                    auto past = this->unconfirmed_message.second;
                    auto now = chrono::steady_clock::now();
                    auto ms = chrono::duration_cast<chrono::milliseconds>(now - past).count();
                    if(new_message.get_ref_id() == unconfirmed_message.first && ms < this->timeout){
                        this->awaiting_confirm = false;
                        this->last_message = MessageUDP(UNKNOWN_MSG, {});
                        this->unconfirmed_message = {};
                    }
                    break;}
                default:
                    break;
            }
            return new_message;
        }
    }
    return MessageUDP(MALFORMED_MSG, {});
}

void ClientUDP::send_bye(){
    if(this->socket_i.get_fd() > 0){
        MessageUDP bye_message = MessageUDP::Builder(BYE_MSG, {})
                                    .set_id(this->message_id)
                                    .set_display_name(this->display_name)
                                    .construct();
        bye_message.build();
        this->socket_i.send(bye_message.get_payload());
        this->last_message = bye_message;
        this->unconfirmed_message = {bye_message.get_id(), chrono::steady_clock::now()};
        this->awaiting_confirm = true;
        this->message_id++;
    } else {
        throw fatal_error(SOCK_NONX, "Failed to accesss UDP socket.");
    }
}

void ClientUDP::send_err(const string& error_message){
    if(this->socket_i.get_fd() > 0){
        MessageUDP err_message = MessageUDP::Builder(ERR_MSG, {})
                                    .set_id(this->message_id)
                                    .set_display_name(this->display_name)
                                    .set_content(error_message)
                                    .construct();
        err_message.build();
        this->socket_i.send(err_message.get_payload());
        this->last_message = err_message;
        this->unconfirmed_message = {err_message.get_id(), chrono::steady_clock::now()};
        this->awaiting_confirm = true;
        this->message_id++;
    } else {
        throw fatal_error(SOCK_NONX, "Failed to accesss UDP socket.");
    }
}

bool ClientUDP::send_msg(const string& text_content){
    if(this->socket_i.get_fd() > 0){
        MessageUDP msg_message = MessageUDP::Builder(MSG_MSG,{})
                                    .set_id(this->message_id)
                                    .set_display_name(this->display_name)
                                    .set_content(text_content)
                                    .construct();
        if(msg_message.build()){
            this->socket_i.send(msg_message.get_payload());
            this->last_message = msg_message;
            this->unconfirmed_message = {msg_message.get_id(), chrono::steady_clock::now()};
            this->awaiting_confirm = true;
            this->message_id++;
            return true;
        } else return false;
    } else {
        throw fatal_error(SOCK_NONX, "Failed to accesss UDP socket.");
    }
}

void ClientUDP::send_confirm(uint16_t ref_id){
    if(this->socket_i.get_fd() > 0){
        MessageUDP confirm_message = MessageUDP::Builder(CONFIRM_MSG,{})
                                    .set_ref_id(ref_id)
                                    .construct();
        if(confirm_message.build()){
            this->socket_i.send(confirm_message.get_payload());
        }
    } else {
        throw fatal_error(SOCK_NONX, "Failed to accesss UDP socket.");
    }
}

FSMState ClientUDP::error_to_server(const string& error_message){
    local_error(error_message); 
    this->send_err(error_message);
    return ENDING;
}

FSMState ClientUDP::send_in_auth(const string& input){
    vector<string> message_parts = tokenize(input);
    if(message_parts.size() != 4){
        local_error("Wrong command format. Expecting '/auth <username> <secret> <display_name>'.");
        return this->fsm_state;
    } else{
        string  command = message_parts[0], 
                username = message_parts[1], 
                secret = message_parts[2], 
                display_name = message_parts[3];
        
        if(command == "/auth"){
            MessageUDP auth_message = MessageUDP::Builder(AUTH_MSG, {})
                                            .set_id(this->message_id)
                                            .set_username(username)
                                            .set_display_name(display_name)
                                            .set_secret(secret)
                                            .construct();
            if(auth_message.build()){
                this->set_display_name(display_name);
                this->socket_i.send(auth_message.get_payload());
                this->last_message = auth_message;
                this->unconfirmed_message = {auth_message.get_id(), chrono::steady_clock::now()};
                this->awaiting_reply = true;
                this->awaiting_confirm = true;
                this->message_id++;
                return AUTH;
            } else{
                local_error("Wrong command format. Expecting '/auth <username> <secret> <display_name>'.");
                return this->fsm_state;
            }
        }
    }
    return this->fsm_state;
}

FSMState ClientUDP::send_in_open(const string& input){
    vector<string> message_parts = tokenize(input);
    if(message_parts.size() == 2 && message_parts[0] == "/join"){
        string command = message_parts[0],
               channel = message_parts[1];

        MessageUDP join_message = MessageUDP::Builder(JOIN_MSG, {})
                                        .set_id(this->message_id)
                                        .set_channel(channel)
                                        .set_display_name(display_name)
                                        .construct();
        if(join_message.build()){
            this->socket_i.send(join_message.get_payload());
            this->last_message = join_message;
            this->unconfirmed_message = {join_message.get_id(), chrono::steady_clock::now()};
            this->awaiting_reply = true;
            this->awaiting_confirm = true;
            this->message_id++;
            return JOIN;
        } else{
            local_error("/join command contains forbidden characters.");
            return this->fsm_state;
        }
    } else{
        if(!this->send_msg(input)){
            local_error("Message you tried to send contains forbidden characters.");
        }
        return this->fsm_state;
    } 
}

FSMState ClientUDP::empty_input_buffer(){
    string earliest_input = this->input_buffer.front();
    this->input_buffer.erase(input_buffer.begin());
    switch(this->fsm_state){
        case START:
            return this->send_in_auth(earliest_input);
            break;
        case AUTH:
            return this->send_in_auth(earliest_input);
            break;
        case OPEN:
            return this->send_in_open(earliest_input);
            break;
        case JOIN:
            if(!this->send_msg(earliest_input)){
                local_error("Message you tried to send contains forbidden characters.");
            }
            return this->fsm_state;
            break;
        default:
            return this->fsm_state;
            break;
    }
}

bool ClientUDP::retransmit_if_timeout(){
    if(awaiting_confirm){
        auto past = this->unconfirmed_message.second;
        auto now = chrono::steady_clock::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(now - past).count();
        if(ms > this->timeout){
            this->socket_i.send(last_message.get_payload());
            this->unconfirmed_message.second = chrono::steady_clock::now();
            this->retry_count += 1;
            if(retry_count > 3){
                local_error("ERROR: Did not receive message confirmation for message '" + last_message.get_printable_payload() +"' in time. Exiting application.");
            }
            return true;
        }
    }
    return false;
}

FSMState ClientUDP::start_state(){
    this->fsm_state = START;
    vector<pollfd> file_descriptors = get_file_descriptors(this->get_socket_i());

    if(!this->awaiting_reply && !this->awaiting_confirm && this->input_buffer.size() > 0){
        this->empty_input_buffer();
    }

    while(true){
        int ret = poll(file_descriptors.data(), file_descriptors.size(), -1);
        if(ret == -1){
            return ENDING;
        }

        if(file_descriptors[0].revents & POLLIN){
            if(this->retransmit_if_timeout()){
                if(this->retry_count > 3){
                    return ENDING;
                }
                return this->fsm_state;
            }

            vector<uint8_t> incoming_payload = this->socket_i.receive(BUFFER_SIZE);
            MessageUDP new_message = this->process_message(incoming_payload);
            if(new_message.get_type() != CONFIRM_MSG){
                this->send_confirm(new_message.get_id());
            }

            switch(new_message.get_type()){
                case ERR_MSG:
                    return ENDING;
                    break;
                case BYE_MSG:
                    return BYE;
                    break;
                case MALFORMED_MSG:
                    return this->error_to_server("Received malformed message from server: " + new_message.get_printable_payload());
                    break;
                case REPLY_MSG:
                case MSG_MSG:
                    return this->error_to_server("Received unexpected message from server: " + new_message.get_printable_payload());
                    break;
                default:
                    break;
            }
        }

        if(file_descriptors[1].revents & POLLIN){
            if(this->retransmit_if_timeout()){
                if(this->retry_count > 3){
                    return ENDING;
                }
                return this->fsm_state;
            }

            string input;
            getline(cin, input);
            if(cin.eof()){
                return BYE;
            }

            if(this->awaiting_confirm || this->awaiting_reply){
                this->input_buffer.push_back(input);
            } else return this->send_in_auth(input);
        }
    }
    return START;
}

FSMState ClientUDP::auth_state(){
    this->fsm_state = AUTH;
    vector<pollfd> file_descriptors = get_file_descriptors(this->get_socket_i());

    if(!this->awaiting_reply && !this->awaiting_confirm && this->input_buffer.size() > 0){
        this->empty_input_buffer();
    }

    while(true){
        int ret = poll(file_descriptors.data(), file_descriptors.size(), -1);
        if(ret == -1){
            return ENDING;
        }

        if(file_descriptors[0].revents & POLLIN){
            if(this->retransmit_if_timeout()){
                if(this->retry_count > 3){
                    return ENDING;
                }
                return this->fsm_state;
            }

            vector<uint8_t> incoming_payload = this->socket_i.receive(BUFFER_SIZE);
            MessageUDP new_message = this->process_message(incoming_payload);
            if(new_message.get_type() != CONFIRM_MSG){
                this->send_confirm(new_message.get_id());
            }

            switch(new_message.get_type()){
                case ERR_MSG:
                    return ENDING;
                    break;
                case BYE_MSG:
                    return BYE;
                    break;
                case MALFORMED_MSG:
                    return this->error_to_server("Received malformed message from server: " + new_message.get_printable_payload());
                    break;
                case REPLY_MSG:
                    this->awaiting_reply = false;
                    if(new_message.get_result() == 1){
                        return OPEN;
                    } else{
                        return AUTH;
                    }
                    break;
                case MSG_MSG:
                    return this->error_to_server("Received unexpected message from server: " + new_message.get_printable_payload());
                    break;
                default:
                    break;
            }
        }

        if(file_descriptors[1].revents & POLLIN){
            if(this->retransmit_if_timeout()){
                if(this->retry_count > 3){
                    return ENDING;
                }
                return this->fsm_state;
            }

            string input;
            getline(cin, input);
            if(cin.eof()){
                return BYE;
            }

            if(this->awaiting_confirm || this->awaiting_reply){
                this->input_buffer.push_back(input);
            } else return this->send_in_auth(input);
        }
    }
    return AUTH;
}

FSMState ClientUDP::open_state(){
    this->fsm_state = OPEN;
    vector<pollfd> file_descriptors = get_file_descriptors(this->get_socket_i());

    if(!this->awaiting_confirm && this->input_buffer.size() > 0){
        this->empty_input_buffer();
    }

    while(true){
        int ret = poll(file_descriptors.data(), file_descriptors.size(), -1);
        if(ret == -1){
            return ENDING;
        }

        if(file_descriptors[0].revents & POLLIN){
            if(this->retransmit_if_timeout()){
                if(this->retry_count > 3){
                    return ENDING;
                }
                return this->fsm_state;
            }

            vector<uint8_t> incoming_payload = this->socket_i.receive(BUFFER_SIZE);
            MessageUDP new_message = this->process_message(incoming_payload);
            if(new_message.get_type() != CONFIRM_MSG){
                this->send_confirm(new_message.get_id());
            }

            switch(new_message.get_type()){
                case ERR_MSG:
                    return ENDING;
                    break;
                case BYE_MSG:
                    return BYE;
                    break;
                case MALFORMED_MSG:
                    return this->error_to_server("Received malformed message from server: " + new_message.get_printable_payload());
                    break;
                case REPLY_MSG:
                    return this->error_to_server("Received unexpected message from server: " + new_message.get_printable_payload());
                    break;
                default:
                    break;
            }
        }

        if(file_descriptors[1].revents & POLLIN){
            if(this->retransmit_if_timeout()){
                if(this->retry_count > 3){
                    return ENDING;
                }
                return this->fsm_state;
            }

            string input;
            getline(cin, input);
            if(cin.eof()){
                return BYE;
            }

            if(this->awaiting_confirm){
                this->input_buffer.push_back(input);
            } else return this->send_in_open(input);
        }
    }
    return OPEN;
}

FSMState ClientUDP::join_state(){
    this->fsm_state = JOIN;
    vector<pollfd> file_descriptors = get_file_descriptors(this->get_socket_i());

    if(!this->awaiting_reply && !this->awaiting_confirm && this->input_buffer.size() > 0){
        this->empty_input_buffer();
    }

    while(true){
        int ret = poll(file_descriptors.data(), file_descriptors.size(), -1);
        if(ret == -1){
            return ENDING;
        }

        if(file_descriptors[0].revents & POLLIN){
            if(this->retransmit_if_timeout()){
                if(this->retry_count > 3){
                    return ENDING;
                }
                return this->fsm_state;
            }

            vector<uint8_t> incoming_payload = this->socket_i.receive(BUFFER_SIZE);
            MessageUDP new_message = this->process_message(incoming_payload);
            if(new_message.get_type() != CONFIRM_MSG){
                this->send_confirm(new_message.get_id());
            }

            switch(new_message.get_type()){
                case ERR_MSG:
                    return ENDING;
                    break;
                case BYE_MSG:
                    return BYE;
                    break;
                case MALFORMED_MSG:
                    return this->error_to_server("Received malformed message from server: " + new_message.get_printable_payload());
                    break;
                case REPLY_MSG:
                    /**
                     * TODO: exit after 5s without reply
                     */
                    this->awaiting_reply = false;
                    return OPEN;
                    break;
                default:
                    break;
            }
        }

        if(file_descriptors[1].revents & POLLIN){
            if(this->retransmit_if_timeout()){
                if(this->retry_count > 3){
                    return ENDING;
                }
                return this->fsm_state;
            }

            string input;
            getline(cin, input);
            if(cin.eof()){
                return BYE;
            }

            if(this->awaiting_reply || this->awaiting_confirm){
                this->input_buffer.push_back(input);
            } else if(!this->send_msg(input)){
                local_error("Message you tried to send contains forbidden characters.");
            }
            return this->fsm_state;
        }
    }
    return JOIN;
}

bool ClientUDP::ending_state(){

}