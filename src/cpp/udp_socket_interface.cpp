/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <string.h>
#include "../header/udp_socket_interface.h"
#include "../header/error.h"

SocketUDP::SocketUDP() 
    : fd(-1), 
      connection({}) {}

void SocketUDP::create(){
    this->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->fd == -1)
        throw fatal_error(SOCK_CREATE, "Failed to open UDP socket.");
}

void SocketUDP::close(){
    ::close(this->fd);
}

int SocketUDP::get_fd(){
    return this->fd;
}

void SocketUDP::set_connection(in_addr_t ip_address, uint16_t port){
    this->connection.sin_family = AF_INET;
    this->connection.sin_port = htons(port);
    this->connection.sin_addr.s_addr = ip_address;
}

void SocketUDP::update_port(uint16_t port){
    this->connection.sin_port = htons(port);
}

sockaddr_in SocketUDP::get_connection() {
    return this->connection;
}

bool SocketUDP::send(const vector<uint8_t>& data){
    ssize_t sent_bytes = sendto(this->fd, data.data(), data.size(), 0, (sockaddr*)&this->connection, sizeof(this->connection));
    if(sent_bytes == -1)
        throw fatal_error(SOCK_SEND, "Failed to send via UDP socket.");
    return true;
}

vector<uint8_t> SocketUDP::receive(int buffer_size) {
    vector<uint8_t> buffer(buffer_size);
    sockaddr_in server_connection;
    socklen_t addr_len = sizeof(sockaddr_in);

    ssize_t received_bytes = recvfrom(this->fd, buffer.data(), buffer_size, 0, (sockaddr*)&server_connection, &addr_len);

    if(ntohs(server_connection.sin_port) != ntohs(this->connection.sin_port)){
        this->connection.sin_port = server_connection.sin_port;
    }

    if(received_bytes == -1)
        throw fatal_error(SOCK_RECV, "Failed to receive from UDP socket.");

    buffer.resize(received_bytes);
    return buffer;
}