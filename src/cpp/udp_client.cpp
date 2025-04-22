/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
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
#include "../header/udp_client.h"
#include "../header/error.h"
#include "../header/string_functions.h"
#include "../header/udp_message.h"

// arbitrarily chosen buffer size for UDP socket reading
#define BUFFER_SIZE 4096

using namespace std;

/**
 * Constructor for UDP Client object
 * @param server hostname or IP address of chat server
 * @param port L4 port
 * @param timeout milisecond amount to wait before message retransmission in case of no message confirmation
 * @param max_retries maximum amount of retransmission attempts before connection is terminated
 * Further sets the default values for class attributes:
 *      display name to 'UNDEFINED', as the client can send a BYE message to the server before setting their display name
 *      FSM state to the initial STARTing state
 *      awaiting reply and confirm to false, as the client is not initially waiting for any messages from the server
 *      last message to an arbitrary value as the class constructor requires it
 *      retry count to 0, as no retransmissions have been attempted yet
 *      require confirm and time to false since the ending state doesnt expect either by default
 */
ClientUDP::ClientUDP(const string &server, uint16_t port, uint16_t timeout, uint8_t max_retries)
    : server(server), port(port), socket_i(), timeout(timeout), max_retries(max_retries), message_id(0),
    display_name("UNDEFINED"), fsm_state(START), awaiting_reply(false), awaiting_confirm(false), last_message(UNKNOWN_MSG, {}), 
    retry_count(0), require_confirm(false), require_time(false) {}

/**
 * Getter/Setter functions
 */
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

/**
 * Creates and returns file descriptors for socket and stdin monitoring
 * @param socket socket to monitor
 * @return vector of pollfd structures
 */
vector<pollfd> get_file_descriptors(SocketUDP socket){
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
 * Simulates a UDP connection, but since UDP is connectionless, only resolves hostname to IP and opens socket
 * @return void
 */
void ClientUDP::verify_address(){
    // create socket
    this->socket_i.create();

    // attempt to resolve hostname
    if(inet_addr(this->server.c_str()) == INADDR_NONE){
        in_addr ip_address = {};
        addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_DGRAM
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
}

/**
 * Closes socket
 * @return FSM state that represents connection termination
 */
void ClientUDP::close_socket(){
    if(this->socket_i.get_fd() > 0){
        this->socket_i.close();
    }
    this->fsm_state = ENDING;
}

/**
 * Processes incoming message
 * @param payload incoming message payload
 * @return processed MessageUDP object
 */
MessageUDP ClientUDP::process_message(const vector<uint8_t> payload){
    // only process valid (not empty) messages
    if(!payload.empty()){
        // create message from payload
        MessageUDP new_message(static_cast<MSG_VAL>(payload[0]), payload);
        if(new_message.parse()){
            // check if message isnt an already processed retransmitted one
            if(new_message.get_type() != CONFIRM_MSG){
                if(this->processed_ids.find(new_message.get_id()) != this->processed_ids.end()){
                    return MessageUDP(ALREADY_PROCESSED, {});
                } else{
                    processed_ids.insert(new_message.get_id());
                }
            }
            // handle message based on type
            switch(new_message.get_type()){
                // print reply to STDOUT
                case REPLY_MSG:{
                    string result = (new_message.get_result() == 1) ? "Success" : "Failure";
                    cout << "Action " << result << ": " << new_message.get_content() << "\n";
                    break;}
                // print server ERROR to STDOUT
                case ERR_MSG:
                    cout << "ERROR FROM " << new_message.get_display_name() << ": " << new_message.get_content() << "\n";
                    break;
                // print chat message to STDOUT
                case MSG_MSG:
                    cout << new_message.get_display_name() << ": " << new_message.get_content() << "\n";
                    break;
                // check valid confirmation
                case CONFIRM_MSG:{
                    auto past = this->unconfirmed_message.second;
                    auto now = chrono::steady_clock::now();
                    auto ms = chrono::duration_cast<chrono::milliseconds>(now - past).count();
                    // CONFIRM message reference ID has to match ID of last sent message
                    // CONFIRM has to arrive within the allowed timeout period, otherwise it is not recognized as a valid confirm
                    if(new_message.get_ref_id() == unconfirmed_message.first && ms < this->timeout){
                        // CONFIRM is valid
                        // stop awaiting a confirm
                        this->awaiting_confirm = false;
                        // "reset" last sent message to a default unused value
                        this->last_message = MessageUDP(UNKNOWN_MSG, {});
                        // reset ID and timestamp of last sent message to default unused value
                        this->unconfirmed_message = {};
                    }
                    break;}
                default:
                    break;
            }

            return new_message;
        }
    }
    // failed message parsing resulting in a malformed message
    return MessageUDP(MALFORMED_MSG, {});
}

/**
 * Sends AUTH message
 * @param username AUTH username
 * @param secret AUTH secret
 * @param display_name AUTH display name
 * @return true if message conforms to grammar
 */
void ClientUDP::send_auth(const string &username, const string &secret, const string &display_name){
    if(this->socket_i.get_fd() > 0){
        // if socket is open
        // create AUTH message
        MessageUDP auth_message = MessageUDP::Builder(AUTH_MSG, {})
                                        .set_id(this->message_id)
                                        .set_username(username)
                                        .set_display_name(display_name)
                                        .set_secret(secret)
                                        .construct();
        if(auth_message.build()){
            // set display name
            this->set_display_name(display_name);
            // send message
            this->socket_i.send(auth_message.get_payload());
            // save the message in case of necessary retransmission
            this->last_message = auth_message;
            // save the message ID and time to verify its confirmation later
            this->unconfirmed_message = {auth_message.get_id(), chrono::steady_clock::now()};
            // start waiting for CONFIRM message
            this->awaiting_confirm = true;
            // start waiting for REPLY message to REQUEST
            this->awaiting_reply = true;
            // save time of sending for REPLY timeout
            this->reply_expect_begin = chrono::steady_clock::now();
            // increment message ID
            this->message_id++;
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
void ClientUDP::send_join(const string &channel){
    if(this->socket_i.get_fd() > 0){
        // if socket is open
        // create JOIN message
        MessageUDP join_message = MessageUDP::Builder(JOIN_MSG, {})
                                    .set_id(this->message_id)
                                    .set_channel(channel)
                                    .set_display_name(this->display_name)
                                    .construct();
        if(join_message.build()){
            // send message
            this->socket_i.send(join_message.get_payload());
            // save the message in case of necessary retransmission
            this->last_message = join_message;
            // save the message ID and time to verify its confirmation later
            this->unconfirmed_message = {join_message.get_id(), chrono::steady_clock::now()};
            // start waiting for CONFIRM message
            this->awaiting_confirm = true;
            // start waiting for REPLY message to REQUEST
            this->awaiting_reply = true;
            // save time of sending for REPLY timeout
            this->reply_expect_begin = chrono::steady_clock::now();
            // increment message ID
            this->message_id++;
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
void ClientUDP::send_bye(){
    if(this->socket_i.get_fd() > 0){
        // if socket is open
        // create BYE message
        MessageUDP bye_message = MessageUDP::Builder(BYE_MSG, {})
                                    .set_id(this->message_id)
                                    .set_display_name(this->display_name)
                                    .construct();
        bye_message.build();
        // send message
        this->socket_i.send(bye_message.get_payload());
        // save the message in case of necessary retransmission
        this->last_message = bye_message;
        // save the message ID and time to verify its confirmation later
        this->unconfirmed_message = {bye_message.get_id(), chrono::steady_clock::now()};
        // start waiting for CONFIRM message
        this->awaiting_confirm = true;
        // increment message ID
        this->message_id++;
    } else {
        // socket isnt open
        throw fatal_error(SOCK_NONX, "Failed to accesss UDP socket.");
    }
}

/**
 * Sends ERR message
 * @param error_message message describing the error
 * @return void
 */
void ClientUDP::send_err(const string& error_message){
    if(this->socket_i.get_fd() > 0){
        // if socket is open
        // create ERR message
        MessageUDP err_message = MessageUDP::Builder(ERR_MSG, {})
                                    .set_id(this->message_id)
                                    .set_display_name(this->display_name)
                                    .set_content(error_message)
                                    .construct();
        err_message.build();
        // send message
        this->socket_i.send(err_message.get_payload());
        // save the message in case of necessary retransmission
        this->last_message = err_message;
        // save the message ID and time to verify its confirmation later
        this->unconfirmed_message = {err_message.get_id(), chrono::steady_clock::now()};
        // start waiting for CONFIRM message
        this->awaiting_confirm = true;
        // increment message ID
        this->message_id++;
    } else {
        // socket isnt open
        throw fatal_error(SOCK_NONX, "Failed to accesss UDP socket.");
    }
}

/**
 * Sends MSG message
 * @param text_content message text content
 * @return void
 */
void ClientUDP::send_msg(const string& text_content){
    if(this->socket_i.get_fd() > 0){
        // if socket is open
        // create message
        MessageUDP msg_message = MessageUDP::Builder(MSG_MSG,{})
                                    .set_id(this->message_id)
                                    .set_display_name(this->display_name)
                                    .set_content(text_content)
                                    .construct();
        if(msg_message.build()){
            // send message
            this->socket_i.send(msg_message.get_payload());
            // save the message in case of necessary retransmission
            this->last_message = msg_message;
            // save the message ID and time to verify its confirmation later
            this->unconfirmed_message = {msg_message.get_id(), chrono::steady_clock::now()};
            // start waiting for CONFIRM message
            this->awaiting_confirm = true;
            // increment message ID
            this->message_id++;
        } else{
            local_error("Message you tried to send contains forbidden characters.");
        }
    } else {
        // socket isnt open
        throw fatal_error(SOCK_NONX, "Failed to accesss UDP socket.");
    }
}

/**
 * Sends CONFIRM message
 * @param ref_id reference ID of message being confirmed
 * @return void
 */
void ClientUDP::send_confirm(uint16_t ref_id){
    if(this->socket_i.get_fd() > 0){
        // if socket is open
        // create message
        MessageUDP confirm_message = MessageUDP::Builder(CONFIRM_MSG,{})
                                    .set_ref_id(ref_id)
                                    .construct();
        if(confirm_message.build()){
            // send message
            this->socket_i.send(confirm_message.get_payload());
        }
    } else {
        // socket isnt open
        throw fatal_error(SOCK_NONX, "Failed to accesss UDP socket.");
    }
}

/**
 * Wrapper to send an ERR message to server and also print the error message locally
 * Results in connection termination
 * @param error_message message to be included in ERR message
 * @return FSM state that terminates connection
 */
FSMState ClientUDP::error_to_server(const string& error_message){
    // print local error to STDOUT
    local_error(error_message); 
    // send ERR message
    this->send_err(error_message);
    // wait for CONFIRM message in connection termination stage
    this->require_confirm = true;
    this->require_time = false;
    return ENDING;
}

/**
 * Checks if an expected REQUEST message reply hasnt timed out
 * @return true if timeout has occured
 */
bool ClientUDP::check_reply(){
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
bool ClientUDP::parse_as_command(const vector<string> &input){
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
FSMState ClientUDP::send_message(bool read_flag, const string &buffer_input){
    string input;
    if(!read_flag){
        // get input from STDIN
        getline(cin, input);
        if(cin.eof()){
            // ctrl+D
            this->require_confirm = true;
            this->require_time = false;
            return BYE;
        }

        if(this->awaiting_confirm || this->awaiting_reply){
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
 * Sends buffered input to server
 * @return next FSM state
 */
FSMState ClientUDP::empty_input_buffer(){
    // get first buffered input, remove it from buffer, send message
    string earliest_input = this->input_buffer.front();
    this->input_buffer.erase(input_buffer.begin());
    return this->send_message(true, earliest_input);
}

/**
 * Retransmits last sent message if no CONFIRM message was received
 * @return true if timeout has occured
 */
bool ClientUDP::retransmit_if_timeout(){
    // client is waiting for a CONFIRM
    if(awaiting_confirm){
        // compare current time to the time the message was sent
        auto past = this->unconfirmed_message.second;
        auto now = chrono::steady_clock::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(now - past).count();
        if(ms > this->timeout){
            // timeout has occured
            // retransmit last sent message
            this->socket_i.send(last_message.get_payload());
            // update the time it was sent
            this->unconfirmed_message.second = chrono::steady_clock::now();
            // increment retransmission counter
            this->retry_count += 1;
            if(retry_count > this->max_retries){
                // print local error if maximum retransmission count has been reached
                // connetion is assumed to be terminated, not waiting for CONFIRM or retransmission
                this->require_confirm = false;
                this->require_time = false;
                local_error("Did not receive message confirmation for message '" + last_message.get_printable_payload() +"' in time. Exiting application.");
                // connection termination takes place elsewhere
            }
            return true;
        }
    }
    return false;
}

/**
 * Main client process, reads user input or incoming message and processes it based on FSM state
 * @param NEW_FSM_STATE updated FSM state passed from main()
 * @return next FSM state
 */
FSMState ClientUDP::state_process(FSMState NEW_FSM_STATE){
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
    if(!this->awaiting_reply && !this->awaiting_confirm && this->input_buffer.size() > 0){
        this->empty_input_buffer();
    }

    // create file descriptors for socket and stdin monitoring
    vector<pollfd> file_descriptors = get_file_descriptors(this->get_socket_i());
    while(true){
        int ret = poll(file_descriptors.data(), file_descriptors.size(), -1);
        if(ret == -1){
            // poll() has failed - assume connection termination
            this->require_confirm = false;
            this->require_time = false;
            return ENDING;
        }

        // data available on socket
        if(file_descriptors[0].revents & POLLIN){
            // retransmit unconfirmed message instead of reading new one
            if(this->retransmit_if_timeout()){
                if(this->retry_count > this->max_retries){
                    // terminate if maximum number of retransmissions has been reached
                    return ENDING;
                }
                return this->fsm_state;
            }

            // get message from socket
            vector<uint8_t> incoming_payload = this->socket_i.receive(BUFFER_SIZE);
            MessageUDP new_message = this->process_message(incoming_payload);
            if(new_message.get_type() != CONFIRM_MSG){
                // CONFIRM received message immediately after receiving it
                this->send_confirm(new_message.get_id());
            }

            MSG_VAL message_type = new_message.get_type();
            // handle error-resulting messages independent of state
            // ERR, BYE and malformed messages always result in connection termination
            switch(message_type){
                case ERR_MSG:
                case BYE_MSG:
                    this->require_time = true;
                    return ENDING;
                    break;
                case MALFORMED_MSG:
                    return this->error_to_server("Received malformed message from server: " + new_message.get_printable_payload());
                    break;
                default:
                    break;
            }

            // handle other message types based on FSM state
            // only remaining message types that need handling that the server should send are REPLY and MSG
            // rest should be caught by MALFORMED earlier
            switch(this->fsm_state){
                // REPLY and MSG are unexpected
                case START:
                    if(message_type == REPLY_MSG || message_type == MSG_MSG){
                        return this->error_to_server("Received unexpected message from server: " + new_message.get_printable_payload());
                    }
                    break;
                // MSG is unexpected
                case AUTH:
                    if(message_type == REPLY_MSG){
                        // stop waiting for reply
                        this->awaiting_reply = false;
                        if(new_message.get_result() == 1){
                            // go to OPEN if successful
                            return OPEN;
                        } else{
                            // stay in AUTH if unsuccessful
                            return AUTH;
                        }
                    }
                    if(message_type == MSG_MSG){
                        return this->error_to_server("Received unexpected message from server: " + new_message.get_printable_payload());
                    }
                    break;
                // REPLY is unexpected
                case OPEN:
                    if(message_type == REPLY_MSG){
                        return this->error_to_server("Received unexpected REPLY message from server: " + new_message.get_printable_payload());
                    }
                    break;
                // everything is expected
                case JOIN:
                    if(message_type == REPLY_MSG){
                        // stop waiting for reply
                        this->awaiting_reply = false;
                        return OPEN;
                    }
                    break;
                default:
                    break;
            }
        }

        // data available on stdin
        if(file_descriptors[1].revents & POLLIN){
            // retransmit unconfirmed message instead of reading new one
            if(this->retransmit_if_timeout()){
                if(this->retry_count > this->max_retries){
                    // terminate if maximum number of retransmissions has been reached
                    return ENDING;
                }
                return this->fsm_state;
            }

            // send message to server
            return this->send_message(false, "");
        }
    }
    // if process hasn't changed state, return the current one
    return this->fsm_state;
}

/**
 * UDP connection termination process
 * @return void
 */
void ClientUDP::ending_state(){
    // file descriptors for poll()
    pollfd pfd;
    pfd.fd = this->socket_i.get_fd();
    pfd.events = POLLIN;

    // if expecting message confirmation
    if(require_confirm){
        // attempt to read confirmation MAX+1 times (1 initial, 3 retransmit)
        for(uint8_t i = 0; i < this->max_retries + 1; i++){
            int ret = poll(&pfd, 1, this->timeout);
            if(ret > 0 && (pfd.revents & POLLIN)){
                // get message from socket
                vector<uint8_t> incoming_payload = this->socket_i.receive(BUFFER_SIZE);
                MessageUDP new_message(static_cast<MSG_VAL>(incoming_payload[0]), incoming_payload);
        
                // check if message is expected CONFIRM message
                if(!incoming_payload.empty()){
                    if(new_message.parse()){
                        if(new_message.get_type() == CONFIRM_MSG){
                            auto past = this->unconfirmed_message.second;
                            auto now = chrono::steady_clock::now();
                            auto ms = chrono::duration_cast<chrono::milliseconds>(now - past).count();
                            if(new_message.get_ref_id() == unconfirmed_message.first && ms < this->timeout){
                                // successfully end connection termination
                                return;
                            // send only MAX amount of times, not +1 because initial message was already sent
                            } else if(i != max_retries){
                                // resend unconfirmed message
                                this->socket_i.send(last_message.get_payload());
                            }
                        }
                    }
                }
            }
        }
    }

    // if expecting server to send the same message multiple times
    if(require_time){
        // attempt to confirm message MAX times
        for(uint8_t i = 0; i < this->max_retries; i++){
            int ret = poll(&pfd, 1, this->timeout);
            if(ret > 0 && (pfd.revents & POLLIN)){
                // get message from socket
                vector<uint8_t> incoming_payload = this->socket_i.receive(BUFFER_SIZE);
                MessageUDP new_message(static_cast<MSG_VAL>(incoming_payload[0]), incoming_payload);
                if(!incoming_payload.empty()){
                    if(new_message.parse()){
                        // send confirmation
                        this->send_confirm(new_message.get_ref_id());
                    }
                }
            } else{
                // socket poll timed out without additional message - CONFIRM delivered successfully
                return;
            }
        }
    }
    local_error("Connection timed out during termination process.");
}