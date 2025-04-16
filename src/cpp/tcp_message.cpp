/**
 * Martin Bozko
 * xbozko01
 * 16.04.2025
 */

#include <string>
#include <optional>
#include "message_values.h"
#include "error.h"
#include "../header/tcp_message.h"

MessageTCP::MessageTCP(MSG_VAL type,
                    const std::optional<std::string>& display_name,
                    const std::optional<std::string>& content,
                    const std::optional<std::string>& username,
                    const std::optional<std::string>& secret,
                    const std::optional<std::string>& channel,
                    const std::optional<std::string>& is_ok)
                : type(type), display_name(display_name), content(content),
                username(username), secret(secret), channel(channel), is_ok(is_ok),
                payload("") {}

void MessageTCP::build(){
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



using namespace std;
