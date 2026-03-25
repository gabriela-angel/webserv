#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <sys/stat.h>
# include <sstream>
# include <dirent.h>
# include <unistd.h>
# include <cstring>

// CHECK IF WE CAN KEEP THIS OR WE SHOULD MAKE UTIL CLASSES/NAMESPACES


//string utils

std::string toUpper(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
		str[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[i])));
	return str;
}

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

// file utils

bool isValidDirectory(const std::string& path) {
	struct stat info;
	return (stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode));
}
 
bool isValidFile(const std::string& path) {
	struct stat info;
	return (stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

bool isEmptyDirectory(const std::string& path) {
	DIR* dir = opendir(path.c_str());
	if (!dir)
		return false;
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
			closedir(dir);
			return false;
		}
	}
	closedir(dir);
	return true;
}

std::string guessMimeType(const std::string& path) {
	size_t dot = path.rfind('.');
	if (dot == std::string::npos)
		return "application/octet-stream";
	std::string ext = path.substr(dot + 1);
	if (ext == "html" || ext == "htm") return "text/html";
	if (ext == "css")                  return "text/css";
	if (ext == "js")                   return "text/javascript";
	if (ext == "json")                 return "application/json";
	if (ext == "xml")                  return "application/xml";
	if (ext == "pdf")                  return "application/pdf";
	if (ext == "zip")                  return "application/zip";
	if (ext == "png")                  return "image/png";
	if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
	if (ext == "gif")                  return "image/gif";
	if (ext == "webp")                 return "image/webp";
	if (ext == "svg")                  return "image/svg+xml";
	if (ext == "ico")                  return "image/x-icon";
	if (ext == "txt")                  return "text/plain";
	if (ext == "mp4")                  return "video/mp4";
	if (ext == "webm")                 return "video/webm";
	if (ext == "mp3")                  return "audio/mpeg";
	if (ext == "wav")                  return "audio/wav";
	if (ext == "woff")                 return "font/woff";
	if (ext == "woff2")                return "font/woff2";
	if (ext == "ttf")                  return "font/ttf";
	return "application/octet-stream";
}

#endif

