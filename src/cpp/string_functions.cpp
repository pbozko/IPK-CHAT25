/**
 * Martin Bozko
 * xbozko01
 * 17.04.2025
 */
#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

string extract_value(const string& payload, const string& start, const string& end){
    /**
     * TODO: malformed message exceptions & handling
     */
    size_t start_index = payload.find(start);
    start_index += start.length();
    size_t end_index = payload.find(end, start_index);
    size_t length = end_index - start_index;

    return payload.substr(start_index, length);
}

vector<string> tokenize(const string &input){
    vector<string> tokens;
    string token;
    istringstream stream(input);

    while(getline(stream, token, ' ')){
        tokens.push_back(token);
    }

    return tokens;
}

string get_full_message(string &stream_buffer){
    const string delimiter = "\r\n";
    size_t position = stream_buffer.find(delimiter);

    if(position != string::npos){
        string full_message = stream_buffer.substr(0, position + delimiter.length());
        stream_buffer = stream_buffer.substr(position + delimiter.length());
        return full_message;
    } else return "";
}

bool check_id(const string &id){
    regex id_regex(R"(^[A-Za-z0-9._-]{1,20}$)");
    if(!regex_match(id, id_regex)){
        return false;
    }
    return true;
}

bool check_secret(const string &secret){
    regex secret_regex(R"(^[A-Za-z0-9_-]{1,128}$)");
    if(!regex_match(secret, secret_regex)){
        return false;
    }
    return true;
}

bool check_content(const string &content){
    regex content_regex(R"(^[\x20-\x7E\r\n]*$)");
    if(!regex_match(content, content_regex)){
        return false;
    }
    return true;
}

bool check_dname(const string &display_name){
    regex dname_regex(R"(^[\x20-\x7E]{1,20}$)");
    if(!regex_match(display_name, dname_regex)){
        return false;
    }
    return true;
}