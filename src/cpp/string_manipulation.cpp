/**
 * Martin Bozko
 * xbozko01
 * 17.04.2025
 */
#include <string>

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