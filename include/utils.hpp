#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <sys/stat.h>

// CHECK IF WE CAN KEEP THIS OR WE SHOULD MAKE UTIL CLASSES
std::string toUpper(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
		str[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[i])));
	return str;
}

bool isValidDirectory(const std::string& path) {
	struct stat info;
	if (stat(path.c_str(), &info) != 0 || !S_ISDIR(info.st_mode))
		return false;
	return true;
}

// ID VALID FILE FUNC

std::string trim(const std::string& str) {
	size_t first = str.find_first_not_of(" \t\r\n");
	if (first == std::string::npos)
		return "";
	size_t last = str.find_last_not_of(" \t\r\n");
	return str.substr(first, last - first + 1);
}

std::string itoa(int num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

#endif

