/**
 * Martin Bozko
 * xbozko01
 * 16.04.2025
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <csignal>
#include "../header/tcp_client.h"
#include "../header/error.h"
#include "../header/string_functions.h"
#include "../header/tcp_message.h"

#define BUFFER_SIZE 256
 
using namespace std;

ClientTCP::ClientTCP(const string &server, uint16_t port)
    : server(server), port(port), socket_i(), display_name("Unknown"), stream_buffer(""), fsm_state(START), awaiting_reply(false) {}

string ClientTCP::get_server(){
    return this->server;
}

uint16_t ClientTCP::get_port(){
    return this->port;
}

SocketTCP ClientTCP::get_socket_i(){
    return this->socket_i;
}

string ClientTCP::get_display_name(){
    return this->display_name;
}

void ClientTCP::set_display_name(const string &new_name){
    this->display_name = new_name;
}

void ClientTCP::connect_to_server(){
    this->socket_i.create();

    if(inet_addr(this->server.c_str()) == INADDR_NONE){
        in_addr ip_address = {};
        addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM
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

    sockaddr_in connection = this->socket_i.get_connection();
    if(connect(this->socket_i.get_fd(), (struct sockaddr*)&connection, sizeof(connection)) == -1){
        throw fatal_error(SERV_CONN, "Failed to connect to server.");
    }
}

void ClientTCP::close_connection(){
    if(this->socket_i.get_fd() > 0){
        shutdown(this->socket_i.get_fd(), SHUT_RDWR);
        close(this->socket_i.get_fd());
    }
    this->fsm_state = ENDING;
}

MessageTCP ClientTCP::process_message(){
    string message = get_full_message(this->stream_buffer);
    if(message != ""){
        size_t position = message.find(' ');
        string type = "";
        if(position != string::npos){
            type = message.substr(0, position);
            MessageTCP new_message(type, message);
            if(new_message.parse()){
                if(new_message.get_type() == "REPLY"){
                    string state = "";
                    if(new_message.get_is_ok() == "OK"){
                        state = "Success";
                    } else{
                        state = "Failure";
                    }
                    cout << "Action " << state << ": " << new_message.get_content() << "\n";
                } else if(new_message.get_type() == "ERR"){
                    cout << "ERROR FROM " << new_message.get_display_name() << ": " << new_message.get_content() << "\n";
                } else if(new_message.get_type() == "MSG" && (this->fsm_state == OPEN || this->fsm_state == JOIN)){
                    cout << new_message.get_display_name() << ": " << new_message.get_content() << "\n";
                }

                return new_message;
            } else{
                return MessageTCP("MALFORMED", "");
            }
        }
    } return MessageTCP("INCOMPLETE", "");
}

void ClientTCP::send_bye(){
    if(this->socket_i.get_fd() > 0){
        MessageTCP bye_message = MessageTCP::Builder("BYE", "")
                                    .set_display_name(this->display_name)
                                    .construct();
        bye_message.build();
        this->socket_i.send(bye_message.get_payload());
    } else {
        throw fatal_error(SOCK_NONX, "Failed to accesss TCP socket.");
    }

}

void ClientTCP::send_err(const string& error_message){
    if(this->socket_i.get_fd() > 0){
        MessageTCP err_message = MessageTCP::Builder("ERR", "")
                                    .set_display_name(this->display_name)
                                    .set_content(error_message)
                                    .construct();
        err_message.build();
        this->socket_i.send(err_message.get_payload());
    } else {
        throw fatal_error(SOCK_NONX, "Failed to accesss TCP socket.");
    }
}

bool ClientTCP::send_msg(const string& text_content){
    if(this->socket_i.get_fd() > 0){
        MessageTCP msg_message = MessageTCP::Builder("MSG", "")
                                    .set_display_name(this->display_name)
                                    .set_content(text_content)
                                    .construct();
        if(msg_message.build()){
            this->socket_i.send(msg_message.get_payload());
            return true;
        } else return false;
    } else {
        throw fatal_error(SOCK_NONX, "Failed to accesss TCP socket.");
    }
}

FSMState ClientTCP::send_in_auth(const string& input){
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
            MessageTCP auth_message = MessageTCP::Builder("AUTH", "")
                                            .set_username(username)
                                            .set_secret(secret)
                                            .set_display_name(display_name)
                                            .construct();
            if(auth_message.build()){
                this->set_display_name(display_name);
                this->socket_i.send(auth_message.get_payload());
                this->awaiting_reply = true;
                return AUTH;
            } else{
                local_error("Wrong command format. Expecting '/auth <username> <secret> <display_name>'.");
                return this->fsm_state;
            }
        }
    }
    return this->fsm_state;
}

FSMState ClientTCP::send_in_open(const string& input){
    vector<string> message_parts = tokenize(input);
    if(message_parts.size() == 2 && message_parts[0] == "/join"){
        string command = message_parts[0],
               channel = message_parts[1];

        MessageTCP join_message = MessageTCP::Builder("JOIN", "")
                                        .set_channel(channel)
                                        .set_display_name(display_name)
                                        .construct();
        if(join_message.build()){
            this->socket_i.send(join_message.get_payload());
            this->awaiting_reply = true;
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

FSMState ClientTCP::read_stream(){
    string new_data = this->socket_i.receive(BUFFER_SIZE);
    if(new_data.empty()){
        local_error("TCP socket closed unexpectedly.");
        return ENDING;
    }
    this->stream_buffer += new_data;
    return this->fsm_state;
}

FSMState ClientTCP::empty_input_buffer(){
    string earliest_input = this->input_buffer.front();
    this->input_buffer.erase(input_buffer.begin());
    switch(this->fsm_state){
        case AUTH:
            return this->send_in_auth(earliest_input);
            break;
        case OPEN:
            return this->send_in_open(earliest_input);
            break;
        default:
            return this->fsm_state;
            break;
    }
}

FSMState ClientTCP::error_to_server(const string& error_message){
    local_error(error_message); 
    this->send_err(error_message);
    return ENDING;
}

vector<pollfd> get_file_descriptors(SocketTCP socket){
    vector<pollfd> fds(2);

    fds[0].fd = socket.get_fd();
    fds[0].events = POLLIN;

    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    return fds;
}

FSMState ClientTCP::start_state(){
    this->fsm_state = START;
    vector<pollfd> file_descriptors = get_file_descriptors(this->get_socket_i());
    while(true){
        int ret = poll(file_descriptors.data(), file_descriptors.size(), -1);
        if(ret == -1){
            return ENDING;
        }

        if(file_descriptors[0].revents & POLLIN){
            if(this->read_stream() == ENDING){
                return ENDING;
            }
            MessageTCP new_message = this->process_message();

            if(new_message.get_type() == "ERR" || new_message.get_type() == "BYE"){
                return ENDING;
            } else if(new_message.get_type() == "INCOMPLETE"){
                return this->fsm_state;
            } else if(new_message.get_type() == "MALFORMED"){
                return this->error_to_server("Received malformed message from server: " + new_message.get_payload());
            } else{
                /**
                 * TODO: make sure an outgoing ERR message should also be printed to stdout.
                 */
                return this->error_to_server("Received unexpected message from server: " + new_message.get_payload());
            }
        }

        if(file_descriptors[1].revents & POLLIN){
            string input;
            getline(cin, input);
            if(cin.eof()){
                return ENDING;
            }

            return this->send_in_auth(input);
        }
    }
    return START;
}

FSMState ClientTCP::auth_state(){
    this->fsm_state = AUTH;
    vector<pollfd> file_descriptors = get_file_descriptors(this->get_socket_i());

    if(!this->awaiting_reply && this->input_buffer.size() > 0){
        this->empty_input_buffer();
    }

    while(true){
        int ret = poll(file_descriptors.data(), file_descriptors.size(), -1);
        if(ret == -1){
            return ENDING;
        }

        if(file_descriptors[0].revents & POLLIN){
            if(this->read_stream() == ENDING){
                return ENDING;
            }
            MessageTCP new_message = this->process_message();

            if(new_message.get_type() == "REPLY"){
                this->awaiting_reply = false;
                if(new_message.get_is_ok() == "OK"){
                    return OPEN;
                } else{ 
                    return AUTH;
                }
            } else if(new_message.get_type() == "MSG"){
                local_error("Received unexpected MSG message.");
                this->send_err("Received unexpected MSG message.");
                return ENDING;
            } else if(new_message.get_type() == "ERR" || new_message.get_type() == "BYE"){
                return ENDING;
            } else if(new_message.get_type() == "INCOMPLETE"){
                return this->fsm_state;
            } else if(new_message.get_type() == "MALFORMED"){
                return this->error_to_server("Received malformed message from server: " + new_message.get_payload());
            }else{
                // shouldnt occur
                return this->error_to_server("Received unexpected message from server: " + new_message.get_payload());
            }
        }

        if(file_descriptors[1].revents & POLLIN){
            string input;
            getline(cin, input);
            if(cin.eof()){
                return ENDING;
            }

            if(this->awaiting_reply){
                this->input_buffer.push_back(input);
            } else return this->send_in_auth(input);
        }
    }
    return AUTH;
}

FSMState ClientTCP::open_state(){
    this->fsm_state = OPEN;
    vector<pollfd> file_descriptors = get_file_descriptors(this->get_socket_i());

    while(true){
        int ret = poll(file_descriptors.data(), file_descriptors.size(), -1);
        if(ret == -1){
            return ENDING;
        }

        if(file_descriptors[0].revents & POLLIN){
            if(this->read_stream() == ENDING){
                return ENDING;
            }
            MessageTCP new_message = this->process_message();

            if(new_message.get_type() == "ERR" || new_message.get_type() == "BYE"){
                return ENDING;
            } else if(new_message.get_type() == "REPLY"){
                local_error("Received unexpected REPLY message from server."); 
                this->send_err("Received unexpected REPLY message from server.");
                return ENDING;
            } else if(new_message.get_type() == "INCOMPLETE"){
                return this->fsm_state;
            } else if(new_message.get_type() == "MALFORMED"){
                return this->error_to_server("Received malformed message from server: " + new_message.get_payload());
            } else if(new_message.get_type() != "MSG"){
                // shouldnt occur
                return this->error_to_server("Received unexpected message from server: " + new_message.get_payload());
            }
        }

        if(file_descriptors[1].revents & POLLIN){
            string input;
            getline(cin, input);
            if(cin.eof()){
                return ENDING;
            }

            return this->send_in_open(input);
        }
    }
    return OPEN;
}

FSMState ClientTCP::join_state(){
    this->fsm_state = JOIN;
    vector<pollfd> file_descriptors = get_file_descriptors(this->get_socket_i());

    if(!this->awaiting_reply && this->input_buffer.size() > 0){
        this->empty_input_buffer();
    }

    while(true){
        int ret = poll(file_descriptors.data(), file_descriptors.size(), -1);
        if(ret == -1){
            return ENDING;
        }

        if(file_descriptors[0].revents & POLLIN){
            if(this->read_stream() == ENDING){
                return ENDING;
            }
            MessageTCP new_message = this->process_message();

            if(new_message.get_type() == "ERR" || new_message.get_type() == "BYE"){
                return ENDING;
            } else if(new_message.get_type() == "REPLY"){
                this->awaiting_reply = false;
                return OPEN;
            } else if(new_message.get_type() == "INCOMPLETE"){
                return this->fsm_state;
            } else if(new_message.get_type() == "MALFORMED"){
                return this->error_to_server("Received malformed message from server: " + new_message.get_payload());
            } else if(new_message.get_type() != "MSG"){
                // shouldnt occur
                return this->error_to_server("Received unexpected message from server: " + new_message.get_payload());
            }
        }

        if(file_descriptors[1].revents & POLLIN){
            string input;
            getline(cin, input);
            if(cin.eof()){
                return ENDING;
            }

            if(this->awaiting_reply){
                this->input_buffer.push_back(input);
            } else if(!this->send_msg(input)){
                local_error("Message you tried to send contains forbidden characters.");
            }
            return this->fsm_state;
        }
    }
    return JOIN;
}