#include "Headers.hpp"
#include "Http.hpp"

/* Ortodoxal-Canonical-Form */
Headers::Headers(const Headers& copy) {
    *this = copy;
}

Headers& Headers::operator=(const Headers& other) {
    if (this != &other) {
        _headers = other._headers;
    }
    return *this;
}

/* ------------------------ */

inline static std::string toUpper(const std::string& str) {
    std::string upper_str;
    for (size_t i = 0; i < str.size(); i++) {
        char c = str[i];
        upper_str += (c >= 'a' && c <= 'z') ? static_cast<char>(toupper(c)) : c;
    }
    return upper_str;
}

inline static std::string TrainCase(const std::string& str) {
    std::string train_str;
    bool capitalize = true;
    for (size_t i = 0; i < str.size(); i++) {
        char c = str[i];
        if (c == '-') {
            train_str += '-';
            capitalize = true;
        } else {
            train_str += capitalize ? static_cast<char>(toupper(c)) : static_cast<char>(tolower(c));
            capitalize = false;
        }
    }
    return train_str;
}


void Headers::add(const std::string& key, const std::string& value) {
    // Uppercase the key for case-insensitive matching
    _headers[toUpper(key)].push_back(value);
}

void Headers::remove(const std::string& key) {
    _headers.erase(toUpper(key));
}

bool Headers::hasKey(const std::string& key) const {
    return _headers.find(toUpper(key)) != _headers.end();
}

Headers::ConstIterator Headers::find(const std::string& key) const {
    return _headers.find(toUpper(key));
}

Headers::Iterator Headers::find(const std::string& key) {
    return _headers.find(toUpper(key));
}

Headers::Iterator Headers::begin() {
    return _headers.begin();
}

Headers::ConstIterator Headers::begin() const {
    return _headers.begin();
}

Headers::Iterator Headers::end() {
    return _headers.end();
}

Headers::ConstIterator Headers::end() const {
    return _headers.end();
}

Headers::Values &Headers::operator[](const std::string& key) {
    return _headers[toUpper(key)];
}

Headers::Values &Headers::at(const std::string& key) {
    return _headers.at(toUpper(key));
}

const Headers::Values &Headers::at(const std::string& key) const {
    return _headers.at(toUpper(key));
}

// Returns Train-Case string of all headers, e.g. "Content-Type: text/html\r\nHost: example.com\r\n"
std::string Headers::toString() const {
    std::string result;
    for (ConstIterator it = _headers.begin(); it != _headers.end(); ++it) {
        const std::string& key = it->first;
        const Values& values = it->second;
        for (size_t i = 0; i < values.size(); i++) {
            result += TrainCase(key) + ": " + values[i] + CRLF;
        }
    }
    return result;
}