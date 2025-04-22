/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 * 
 * TCP client class implementation
 */

/**
 * Headers
 */
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include "../header/tcp_client.h"
#include "../header/error.h"
#include "../header/string_functions.h"
#include "../header/tcp_message.h"

// arbitrarily chosen buffer size for TCP stream reading
#define BUFFER_SIZE 256
 
using namespace std;

/**
 * Constructor for TCP Client object
 * @param server hostname or IP address of chat server
 * @param port L4 port
 * Further sets the default values for class attributes:
 *      display name to 'UNDEFINED', as the client can send a BYE message to the server before setting their display name
        FSM state to the initial STARTing state
        awaiting reply to false, as the client is not initially waiting for any messages from the server
 */     
ClientTCP::ClientTCP(const string &server, uint16_t port)
    : server(server), port(port), socket_i(), display_name("UNDEFINED"), 
    fsm_state(START), awaiting_reply(false) {}

/**
 * Getter/Setter functions
 */
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

/**
 * Creates and returns file descriptors for socket and stdin monitoring
 * @param socket socket to monitor
 * @return vector of pollfd structures
 */
vector<pollfd> get_file_descriptors(SocketTCP socket){
    vector<pollfd> fds(2);

    // monitor socket
    fds[0].fd = socket.get_fd();
    fds[0].events = POLLIN;

    // monitor stdin
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    return fds;
}

/**
 * Creates TCP socket and establishes connection with server
 * @return void
 */
void ClientTCP::connect_to_server(){
    // create socket
    this->socket_i.create();

    // attempt to resolve hostname
    if(inet_addr(this->server.c_str()) == INADDR_NONE){
        in_addr ip_address = {};
        addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM
        };

        // struct to store resolved IP info
        addrinfo *resolved = nullptr;
        int result = getaddrinfo(this->server.c_str(), nullptr, &hints, &resolved);
        if(result != 0 || resolved == nullptr){
            if(resolved){
                freeaddrinfo(resolved);
            }
                throw fatal_error(SERV_RESL, "Failed to resolve hostname to IP address.");
        }
        
        // create socket connection
        ip_address = ((struct sockaddr_in*)resolved->ai_addr)->sin_addr;
        this->socket_i.set_connection(ip_address.s_addr, this->port);

        freeaddrinfo(resolved);
    } else{
        // create socket connection
        this->socket_i.set_connection(inet_addr(this->server.c_str()), this->port);
    }

    // connect to server
    sockaddr_in connection = this->socket_i.get_connection();
    if(connect(this->socket_i.get_fd(), (struct sockaddr*)&connection, sizeof(connection)) == -1){
        throw fatal_error(SERV_CONN, "Failed to connect to server.");
    }
}

/**
 * Closes socket
 * @return FSM state that represents connection termination
 */
void ClientTCP::close_connection(){
    if(this->socket_i.get_fd() > 0){
        shutdown(this->socket_i.get_fd(), SHUT_RDWR);
        this->socket_i.close();
    }
    this->fsm_state = ENDING;
}

/**
 * Processes incoming message
 * @return processed MessageTCP object
 */
MessageTCP ClientTCP::process_message(){
    // obtain full message from TCP stream
    string message = get_full_message(this->stream_buffer);
    if(message != ""){
        // obtain message type
        size_t position = message.find(' ');
        string type = "";
        // message contains type
        if(position != string::npos){
            type = message.substr(0, position);
            MessageTCP new_message(type, message);
            // fill message attributes from payload
            if(new_message.parse()){
                if(new_message.get_type() == "REPLY"){
                    // print REPLY message
                    string result = (new_message.get_is_ok() == "OK") ? "Success" : "Failure";
                    cout << "Action " << result << ": " << new_message.get_content() << "\n";
                } else if(new_message.get_type() == "ERR"){
                    // print ERR message
                    cout << "ERROR FROM " << new_message.get_display_name() << ": " << new_message.get_content() << "\n";
                } else if(new_message.get_type() == "MSG" && (this->fsm_state == OPEN || this->fsm_state == JOIN)){
                    // print MSG message
                    cout << new_message.get_display_name() << ": " << new_message.get_content() << "\n";
                }

                return new_message;
            } else{
                // message doesnt contain type or failed message parsing resulting in a malformed message
                return MessageTCP("MALFORMED", "");
            }
        }
    } 
    // TCP stream doesnt include a full message
    return MessageTCP("INCOMPLETE", "");
}

/**
 * Sends AUTH message
 * @param username AUTH username
 * @param secret AUTH secret
 * @param display_name AUTH display name
 * @return true if message conforms to grammar
 */
void ClientTCP::send_auth(const string &username, const string &secret, const string &display_name){
    if(this->socket_i.get_fd() > 0){
        // if socket is open
        // create AUTH message
        MessageTCP auth_message = MessageTCP::Builder("AUTH", "")
                                        .set_username(username)
                                        .set_display_name(display_name)
                                        .set_secret(secret)
                                        .construct();
        if(auth_message.build()){
            // set display name
            this->set_display_name(display_name);
            // send message
            this->socket_i.send(auth_message.get_payload());
            // start waiting for REPLY message to REQUEST
            this->awaiting_reply = true;
            // save time of sending for REPLY timeout
            this->reply_expect_begin = chrono::steady_clock::now();
            // set "next" FSM state
            this->fsm_state = AUTH;
        } else{
            local_error("Wrong '/auth' command format. See '/help'.");
        }
    } else{
        // socket isnt open
        throw fatal_error(SOCK_NONX, "Failed to accesss TCP socket.");
    }
}

/**
 * Sends JOIN message
 * @param channel JOIN channel
 * @return true if message conforms to grammar
 */
void ClientTCP::send_join(const string &channel){
    if(this->socket_i.get_fd() > 0){
        // if socket is open
        // create JOIN message
        MessageTCP join_message = MessageTCP::Builder("JOIN", "")
                                    .set_channel(channel)
                                    .set_display_name(this->display_name)
                                    .construct();
        if(join_message.build()){
            // send message
            this->socket_i.send(join_message.get_payload());
            // start waiting for REPLY message to REQUEST
            this->awaiting_reply = true;
            // save time of sending for REPLY timeout
            this->reply_expect_begin = chrono::steady_clock::now();
            // set "next" FSM state
            this->fsm_state = JOIN;
        } else{
            local_error("Wrong '/join' command format. See '/help'.");
        }
    } else {
        // socket isnt open
        throw fatal_error(SOCK_NONX, "Failed to accesss TCP socket.");
    }
}

/**
 * Sends BYE message
 * @return void
 */
void ClientTCP::send_bye(){
    if(this->socket_i.get_fd() > 0){
        // if socket is open
        // create BYE message
        MessageTCP bye_message = MessageTCP::Builder("BYE", "")
                                    .set_display_name(this->display_name)
                                    .construct();
        bye_message.build();
        // send message
        this->socket_i.send(bye_message.get_payload());
    } else {
        // socket isnt open
        throw fatal_error(SOCK_NONX, "Failed to accesss TCP socket.");
    }
}

/**
 * Sends ERR message
 * @param error_message message to be included in ERR message
 * @return void
 */
void ClientTCP::send_err(const string &error_message){
    if(this->socket_i.get_fd() > 0){
        // if socket is open
        // create message
        MessageTCP err_message = MessageTCP::Builder("ERR", "")
                                    .set_display_name(this->display_name)
                                    .set_content(error_message)
                                    .construct();
        err_message.build();
        // send message
        this->socket_i.send(err_message.get_payload());
    } else {
        // socket isnt open
        throw fatal_error(SOCK_NONX, "Failed to accesss TCP socket.");
    }
}

/**
 * Sends MSG message
 * @param text_content message text content
 * @return void
 */
void ClientTCP::send_msg(const string &text_content){
    if(this->socket_i.get_fd() > 0){
        // if socket is open
        // create message
        MessageTCP msg_message = MessageTCP::Builder("MSG", "")
                                    .set_display_name(this->display_name)
                                    .set_content(text_content)
                                    .construct();
        if(msg_message.build()){
            // send message
            this->socket_i.send(msg_message.get_payload());
        } else{
            local_error("Message you tried to send contains forbidden characters.");
        }
    } else {
        // socket isnt open
        throw fatal_error(SOCK_NONX, "Failed to accesss TCP socket.");
    }
}

/**
 * Wrapper to send an ERR message to server and also print the error message locally
 * Results in connection termination
 * @param error_message message to be included in ERR message
 * @return FSM state that terminates connection
 */
FSMState ClientTCP::error_to_server(const string &error_message){
    local_error(error_message); 
    this->send_err(error_message);
    return ENDING;
}

/**
 * Checks if an expected REQUEST message reply hasnt timed out
 * @return true if timeout has occured
 */
bool ClientTCP::check_reply(){
    // client is awaiting reply
    if(awaiting_reply){
        // compare current time to the time the request was sent
        auto past = this->reply_expect_begin;
        auto now = chrono::steady_clock::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(now - past).count();
        // timeout if 5 seconds have passed
        if(ms > 5000){
            return true;
        }
    }
    return false;
}

/**
 * Checks to see if passed input is a command
 * Executes command if appropriate
 * @param input input to be checked
 * @return true if input starts with '/' (is a command)
 */
bool ClientTCP::parse_as_command(const vector<string> &input){
    // input has a valid size
    if(input.size() > 0){
        // command must not have additional arugments
        if(input[0] == "/rename" && input.size() == 2){
            // display name must conform to grammar
            if(check_dname(input[1])){
                // set display name
                this->display_name = input[1];
            }
            return true;
        // command must not have additional arugments
        } else if(input[0] == "/help" && input.size() == 1){
            cout    << "Available commands: " << endl
                    << "/auth <username> <secret> <display_name>    - log in to chat server." << endl
                    << "/join <channel_id>                          - change chat channel." << endl
                    << "/rename <display_name>                      - changes session display name." << endl
                    << "/help                                       - print this help message." << endl
                    << "Ctrl + C                                    - send BYE to server and exit." << endl
                    << "Ctrl + D                                    - send BYE to server and exit." << endl;
            return true;
        // command must not have additional arugments
        } else if(input[0] == "/auth" && input.size() == 4){
            // command only allowed in START or AUTH state
            if(this->fsm_state == START || this->fsm_state == AUTH){
                this->send_auth(input[1], input[2], input[3]);
            } else{
                local_error("'/auth' command not available in current state.");
            }
            return true;
        // command must not have additional arugments
        } else if(input[0] == "/join" && input.size() == 2){
            // command only allowed in OPEN state
            if(this->fsm_state == OPEN){
                this->send_join(input[1]);
            } else{
                local_error("'/join' command not available in current state.");
            }
            return true;
        // unrecognized command
        } else if(input[0][0] == '/'){
            local_error("Unrecognized command. See '/help'.");
            return true;
        }
    }
    return false;
}

/**
 * Sends message to server
 * @return next FSM state
 */
FSMState ClientTCP::send_message(bool read_flag, const string &buffer_input){
    string input;
    if(!read_flag){
        // get input from STDIN
        getline(cin, input);
        if(cin.eof()){
            // ctrl+D
            return BYE;
        }

        if(this->awaiting_reply){
            this->input_buffer.push_back(input);
            return this->fsm_state;
        }
    } else{
        input = buffer_input;
    }

    // see if input is a command, send command if appropriate
    if(!parse_as_command(tokenize(input))){
        // send MSG message if it adheres to grammar
        this->send_msg(input);
    }
    // current FSM state has been changed to next FSM state -> pass it to main()
    return this->fsm_state;
}

/**
 * Wrapper to read from TCP stream and save data to a buffer
 * @return next FSM state
 */
FSMState ClientTCP::read_stream(){
    string new_data = this->socket_i.receive(BUFFER_SIZE);
    if(new_data.empty()){
        // terminate connection if socket is closed
        local_error("TCP socket closed unexpectedly.");
        return ENDING;
    }
    this->stream_buffer += new_data;
    return this->fsm_state;
}

/**
 * Sends buffered input to server
 * @return next FSM state
 */
FSMState ClientTCP::empty_input_buffer(){
    // get first buffered input, remove it from buffer, send message
    string earliest_input = this->input_buffer.front();
    this->input_buffer.erase(input_buffer.begin());
    return this->send_message(true, earliest_input);
}

/**
 * Main client process, reads user input or incoming message and processes it based on FSM state
 * @param NEW_FSM_STATE updated FSM state passed from main()
 * @return next FSM state
 */
FSMState ClientTCP::state_process(FSMState NEW_FSM_STATE){
    // set current FSM state passed from main()
    this->fsm_state = NEW_FSM_STATE;

    // check if reply hasnt timed out
    if(this->check_reply()){
        // try block because the server might not be available anymore
        try{
            this->error_to_server("Received no response, terminating connection.");
            return ENDING;
        } catch(const std::exception& e){
            return ENDING;
        }
    }

    // empty message buffer if possible
    if(!this->awaiting_reply && this->input_buffer.size() > 0){
        return this->empty_input_buffer();
    }

    // create file descriptors for socket and stdin monitoring
    vector<pollfd> file_descriptors = get_file_descriptors(this->get_socket_i());
    while(true){
        int ret = poll(file_descriptors.data(), file_descriptors.size(), -1);
        if(ret == -1){
            // poll() has failed - assume connection termination
            return ENDING;
        }

        // data available on socket
        if(file_descriptors[0].revents & POLLIN){
            // read TCP stream
            if(this->read_stream() == ENDING){
                return ENDING;
            }
            // get message from TCP stream
            MessageTCP new_message = this->process_message();

            // handle error-resulting messages independent of state
            // ERR, BYE and malformed messages always result in connection termination
            // INCOMPLETE messages should be ignored
            if(new_message.get_type() == "ERR" || new_message.get_type() == "BYE"){
                return ENDING;
            } else if(new_message.get_type() == "INCOMPLETE"){
                return this->fsm_state;
            } else if(new_message.get_type() == "MALFORMED"){
                return this->error_to_server("Received malformed message from server: " + new_message.get_payload());
            }

            // handle other message types based on FSM state
            // only remaining message types that need handling that the server should send are REPLY and MSG
            // rest should be caught by MALFORMED earlier
            switch(this->fsm_state){
                // START expects only ERR or BYE
                case START:
                    return this->error_to_server("Received unexpected message from server: " + new_message.get_payload());
                    break;
                // AUTH expects only ERR, BYE or REPLY
                case AUTH:
                    if(new_message.get_type() == "REPLY"){
                        // stop waiting for reply
                        this->awaiting_reply = false;
                        if(new_message.get_is_ok() == "OK"){
                            // go to OPEN if successful
                            return OPEN;
                        } else{ 
                            // stay in AUTH if unsuccessful
                            return AUTH;
                        }
                    } else{
                        return this->error_to_server("Received unexpected message from server: " + new_message.get_payload());
                    }
                    break;
                case OPEN:
                    if(new_message.get_type() == "REPLY"){
                        this->error_to_server("Received unexpected REPLY message from server.");
                    } else if(new_message.get_type() != "MSG"){
                        return this->error_to_server("Received unexpected message from server: " + new_message.get_payload());
                    }
                case JOIN:
                    if(new_message.get_type() == "REPLY"){
                        this->awaiting_reply = false;
                        return OPEN;
                    } else if(new_message.get_type() != "MSG"){
                        return this->error_to_server("Received unexpected message from server: " + new_message.get_payload());
                    }
                    break;
                default:
                    break;
            }
        }

        // data available on stdin
        if(file_descriptors[1].revents & POLLIN){
            // send message to server
            return this->send_message(false, "");
        }
    }
    // if process hasn't changed state, return the current one
    return this->fsm_state;
}