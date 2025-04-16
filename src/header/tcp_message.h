/**
 * Martin Bozko
 * xbozko01
 * 16.04.2025
 */

#ifndef MESSAGE_UDP_H
#define MESSAGE_UDP_H

#include <string>
#include <optional>
#include <regex>
#include "message_values.h"
#include "error.h"

using namespace std;

class MessageTCP {
    public:
        MessageTCP(MSG_VAL type,
                const optional<string>& display_name = nullopt,
                const optional<string>& content = nullopt,
                const optional<string>& username = nullopt, 
                const optional<string>& secret = nullopt,
                const optional<string>& channel = nullopt,
                const optional<string>& is_ok = nullopt)
            : type(type), display_name(display_name), content(content),
              username(username), secret(secret), channel(channel), is_ok(is_ok), 
              payload("") {}

        void build(){
            /**
             * TODO: check if passed params are valid
             */

            switch(type){
                case ERR:
                    if(display_name && content){
                        payload = "ERR FROM " + display_name.value() + " IS " + content.value();
                    } //else throw error(MISSING_ATT);
                    /**
                     * TODO: what does missing attribute result in?
                     */
                    break;
                case REPLY:
                    if(content && is_ok){
                        payload = "REPLY " + is_ok.value() + " IS " + content.value();
                    }
                    break;
                case AUTH:
                    if(username && display_name && secret){
                        payload = "AUTH " + username.value() + " AS " + display_name.value() + " USING " + secret.value();
                    }
                    break;
                case JOIN:
                    if(channel && display_name){
                        payload = "JOIN " + channel.value() + " AS " + display_name.value();
                    } 
                    break;
                case MSG:
                    if(display_name && content){
                        payload = "MSG FROM " + display_name.value() + " IS " + content.value();
                    }
                    break;
                case BYE:
                    if(display_name){
                        payload = "BYE FROM " + display_name.value();
                    }
                    break;
                default:
                    /**
                     * TODO: what is default unexpected behavior?
                     */
            }

            payload += "\r\n";
        }

        

    private:
        MSG_VAL type;
        optional<string> display_name;
        optional<string> content;
        optional<string> username;
        optional<string> secret;
        optional<string> channel;
        optional<string> is_ok;
        string payload;
};

#endif