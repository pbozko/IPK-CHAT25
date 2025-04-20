/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 */
#ifndef STRING_FUNCTIONS_H
#define STRING_FUNCTIONS_H

#include <string>

using namespace std;

string extract_value(const string& payload, const string& start, const string& end);

vector<string> tokenize(const string &input);

string get_full_message(string &stream_buffer);

bool check_id(const string &id);

bool check_secret(const string &secret);

bool check_content(const string &content);

bool check_dname(const string &display_name);

#endif