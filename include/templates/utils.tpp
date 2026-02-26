#pragma once

#include <sstream>
#include <vector>

/*
	Transform a value to a string.

	Risks:
	- If the type T does not have an overloaded operator<<, this will fail to compile.
*/
template<typename T>
std::string to_string(const T& value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

/*
	Transform a string to a value of type T.

	Risks:
	- If the string cannot be converted to type T, this will return an uninitialized value of T.
	- If the string contains extra characters after the value, they will be ignored.
	- If the type T does not have an overloaded operator>>, this will fail to compile.
*/
template<typename T>
T from_string(const std::string& str) {
	std::stringstream ss(str);
	T value = 0;
	ss >> value;
	return value;
}


/*
	Split a string by a given delimiter and return a vector of tokens.

	Risks:
	- If the type T cannot be safely cast to char, this may produce unexpected results.
*/
template<typename T>
std::vector<std::string> split(const std::string& str, T delimiter) {
	char delim = static_cast<char>(delimiter);
    std::vector<std::string> tokens;
    size_t start = 0, pos;
    while ((pos = str.find(delim, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, pos - start));
        start = pos + 1;
    }
    tokens.push_back(str.substr(start));
    return tokens;
}


/*
	Trim leading and trailing whitespace from a string.

	Risks:
	- If the type T cannot be safely cast to char, this may produce unexpected results.
*/
template<typename T>
std::string trim(const T& str) {
	std::string token(str);
	size_t tokenStart = token.find_first_not_of(" \t");
	size_t tokenEnd = token.find_last_not_of(" \t");
	if (tokenStart == std::string::npos) {
		token.clear(); // Token is all whitespace
	} else {
		token = token.substr(tokenStart, tokenEnd - tokenStart + 1);
	}
	return token;
}
