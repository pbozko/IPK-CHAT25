/**
 * Martin Bozko
 * xbozko01
 * 18.11.2024 for ISA project
 * 20.04.2025 updated for IPK project
 * 
 * TCP Socket interface class implementation
 */

/**
 * Headers
 */
#include <sys/socket.h>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>
#include "../header/tcp_socket_interface.h"
#include "../header/error.h"

/**
 * Constructor for socket interface class
 * Initializes default socket values
 * @return void
 */
SocketTCP::SocketTCP() 
    : fd(-1), 
    connection({}) {}

/**
 * Initializes socket as IPv4 socket stream
 * @return void
 */
void SocketTCP::create(){
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if(this->fd == - 1) 
        throw fatal_error(SOCK_CREATE, "Failed to open TCP socket."); 
}

/**
 * Closes socket
 * @return void
 */
void SocketTCP::close(){
    ::close(this->fd);
}

/**
 * @return socket file descriptor
 */
int SocketTCP::get_fd(){
    return this->fd;
}

/**
 * Connects socket to IPv4 address and port
 * @param ip_address IPv4 address structure
 * @param port port number
 * @return void
 */
void SocketTCP::set_connection(in_addr_t ip_address, uint16_t port){
    this->connection.sin_addr.s_addr = ip_address;
    this->connection.sin_port = htons(port);
    this->connection.sin_family = AF_INET;
}

/**
 * @return socket connection structure
 */
sockaddr_in SocketTCP::get_connection(){
    return this->connection;
}

/**
 * Sends data to server through socket connection
 * @param data data to send
 * @returns true if successful
 */
bool SocketTCP::send(const string &data){
    if(::send(this->fd, data.c_str(), data.length(), 0) == -1) 
        throw fatal_error(SOCK_CREATE, "Failed to send through TCP socket."); 
    return true;
}

/**
 * Receive data from server through socket connection
 * @param buffer_size max size of data to receive
 * @return string received data
 */
string SocketTCP::receive(int buffer_size){
    // string to store data
    string buffer(buffer_size, '\0');

    // store number of received bytes
    int received_bytes = recv(this->fd, &buffer[0], buffer_size, 0);
    
    if(received_bytes == -1) 
        throw fatal_error(SOCK_CREATE, "Failed to receive from TCP socket."); 
    
    buffer.resize(received_bytes);
    return buffer;
}