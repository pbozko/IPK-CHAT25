/**
 * Martin Bozko
 * xbozko01
 * 17.04.2025
 * 
 * Helper functions, mainly for string operations
 */
#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

/**
 * Extracts substring from string
 * @param payload Protocol message payload
 * @param start The lower boundary of the extracted string
 * @param end The upper boundary of the extracted string
 * @return substring located between start and end
 */
string extract_value(const string& payload, const string& start, const string& end){
    size_t start_index = payload.find(start);
    start_index += start.length();
    size_t end_index = payload.find(end, start_index);
    size_t length = end_index - start_index;

    return payload.substr(start_index, length);
}

/**
 * Converts an input string into a vector of strings based on ' ' delimiter. Used for client command parsing.
 * @param input The input string to be parsed
 * @return vector of parsed strings
 */
vector<string> tokenize(const string &input){
    vector<string> tokens;
    string token;
    istringstream stream(input);

    while(getline(stream, token, ' ')){
        tokens.push_back(token);
    }

    return tokens;
}

/**
 * Attempts to read an entire protocol TCP message from the input string
 * @param stream_buffer string containing the unprocessed TCP stream
 * @return whole protocol TCP message with '\r\n' delimiter. If no delimiter was found, returns empty string
 */
string get_full_message(string &stream_buffer){
    const string delimiter = "\r\n";
    size_t position = stream_buffer.find(delimiter);

    if(position != string::npos){
        string full_message = stream_buffer.substr(0, position + delimiter.length());
        stream_buffer = stream_buffer.substr(position + delimiter.length());
        return full_message;
    } else return "";
}

/**
 * Regular expressions to check TCP message values against the defined grammar
 * @return true if regex match was successful
 */
bool check_id(const string &id){
    regex id_regex(R"(^[A-Za-z0-9._-]{1,20}$)");
    return regex_match(id, id_regex);
}

bool check_secret(const string &secret){
    regex secret_regex(R"(^[A-Za-z0-9_-]{1,128}$)");
    return regex_match(secret, secret_regex);
}

bool check_content(const string &content){
    regex content_regex(R"(^[\x20-\x7E\r\n]*$)");
    return regex_match(content, content_regex);
}

bool check_dname(const string &display_name){
    regex dname_regex(R"(^[\x20-\x7E]{1,20}$)");
    return regex_match(display_name, dname_regex);
}