/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 * 
 * UDP Socket interface class implementation
 */

/**
 * Headers
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <string.h>
#include "../header/udp_socket_interface.h"
#include "../header/error.h"

/**
 * Constructor for socket interface class
 * Initializes default socket values
 * @return void
 */
SocketUDP::SocketUDP() 
    : fd(-1), 
      connection({}) {}

      /**
 * Initializes socket for IPv4 datagrams 
 * @return void
 */
void SocketUDP::create(){
    this->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->fd == -1)
        fatal_error(SOCK_CREATE, "Failed to open UDP socket.");
}

/**
 * Closes socket
 * @return void
 */
void SocketUDP::close(){
    ::close(this->fd);
}

/**
 * @return socket file descriptor
 */
int SocketUDP::get_fd(){
    return this->fd;
}

/**
 * Connects socket to IPv4 address and port
 * @param ip_address IPv4 address structure
 * @param port port number
 * @return void
 */
void SocketUDP::set_connection(in_addr_t ip_address, uint16_t port){
    this->connection.sin_family = AF_INET;
    this->connection.sin_port = htons(port);
    this->connection.sin_addr.s_addr = ip_address;
}

/**
 * Updates connection port
 * @param port new port value
 * @return void
 */
void SocketUDP::update_port(uint16_t port){
    this->connection.sin_port = htons(port);
}

/**
 * @return socket connection structure
 */
sockaddr_in SocketUDP::get_connection() {
    return this->connection;
}

/**
 * Sends data to server through socket connection
 * @param data data to send
 * @returns true if successful
 */
bool SocketUDP::send(const vector<uint8_t>& data){
    ssize_t sent_bytes = sendto(this->fd, data.data(), data.size(), 0, (sockaddr*)&this->connection, sizeof(this->connection));
    if(sent_bytes == -1)
        fatal_error(SOCK_SEND, "Failed to send via UDP socket.");
    return true;
}

/**
 * Receive data from server through socket connection
 * @param buffer_size max size of data to receive
 * @return string received data
 */
vector<uint8_t> SocketUDP::receive(int buffer_size){
    // vector of uint8_t to store data
    vector<uint8_t> buffer(buffer_size);
    // variable to store data about the sender
    sockaddr_in server_connection;
    socklen_t addr_len = sizeof(sockaddr_in);

    ssize_t received_bytes = recvfrom(this->fd, buffer.data(), buffer_size, 0, (sockaddr*)&server_connection, &addr_len);

    if(received_bytes == -1)
        fatal_error(SOCK_RECV, "Failed to receive from UDP socket.");

    // update port
    if(ntohs(server_connection.sin_port) != ntohs(this->connection.sin_port)){
        this->connection.sin_port = server_connection.sin_port;
    }

    buffer.resize(received_bytes);
    return buffer;
}