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
#include "../header/tcp_client.h"
#include "../header/error.h"
#include "../header/string_functions.h"
#include "../header/tcp_message.h"

// Client FSM states
#define START 1
#define AUTH 2
#define OPEN 3
#define JOIN 4
#define END 0
 
using namespace std;

ClientTCP::ClientTCP(const string &server, uint16_t port)
    : server(server), port(port), socket_i({}), display_name("") {}

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
        if(result != 0 || resolved == nullptr) 
            //throw error(HOST_RES);
        
        ip_address = ((struct sockaddr_in*)resolved->ai_addr)->sin_addr;
        this->socket_i.set_connection(ip_address.s_addr, this->port);

        freeaddrinfo(resolved);
    } else{
        this->socket_i.set_connection(inet_addr(this->server.c_str()), this->port);
    }

    sockaddr_in connection = this->socket_i.get_connection();
    if(connect(this->socket_i.get_fd(), (struct sockaddr*)&connection, sizeof(connection)) == -1)
        ;// throw error(SERVER_CONN);
}

vector<pollfd> get_file_descriptors(SocketTCP socket){
    vector<pollfd> fds(2);

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = socket.get_fd();
    fds[1].events = POLLIN;

    return fds;
}

int ClientTCP::authenticate(){
    vector<pollfd> file_descriptors = get_file_descriptors(this->get_socket_i());

    while(true){
        int ret = poll(file_descriptors.data(), file_descriptors.size(), -1);
        if(ret == -1){
            // throw fatal error
            break;
        }

        if(file_descriptors[0].revents & POLLIN){
            string input;
            getline(cin, input);

            vector<string> message_parts = tokenize(input);
            if(message_parts.size() != 4){
                // local error
            } else{
                string  command = message_parts[0], 
                        username = message_parts[1], 
                        secret = message_parts[2], 
                        display_name = message_parts[3];

                if(command == "/auth" && check_id(username) && check_secret(secret) && check_dname(display_name)){
                    cout << "6" << endl;
                    MessageTCP auth_message = MessageTCP::Builder("AUTH", "")
                                                    .set_username(username)
                                                    .set_secret(secret)
                                                    .set_display_name(display_name)
                                                    .construct();
                    auth_message.build();
                    auth_message.dump();

                    //this->socket_i.send(auth_message.get_payload());

                    return AUTH;
                }
            }
        }
    }

    return 0;
}

/**
 *             istringstream iss(input);
            string command, username, secret, display_name, unexpected;
            if(!(iss >> command >> username >> secret >> display_name)){
                // throw local error
                cout << "local error: wrong auth format" << endl;
            }
            if(iss >> unexpected){
                cout << "local error: wrong auth format" << endl;
            }
            if(command != "/auth"){
                cout << "local error: wrong auth format" << endl;
            }
 */