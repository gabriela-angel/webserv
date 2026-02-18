#include "HttpRequest.hpp"

HttpRequest::HttpRequest() {}
HttpRequest::~HttpRequest() {}

void HttpRequest::updateState(ClientData &client) {

	long headerSize = _headerSize(client.readBuffer);
	if (headerSize == -1)  // Header not fully received
	{
		client.readState = READING;
		return;
	}

	long contentLength = _contentLength(client.readBuffer);
	if (contentLength == -1)  // Malformed Content-Length header
	{
		client.readState = ERROR;
		return;
	}

	if (client.readBuffer.size() >= static_cast<size_t>(headerSize + contentLength))
		client.readState = READ_COMPLETE;
	else
		client.readState = READING;
}


long HttpRequest::_headerSize(const std::string &buffer) {
	size_t pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos) return -1;
	return pos + 4; // Include the size of the delimiter
}


long HttpRequest::_contentLength(const std::string &buffer) {
	size_t pos = buffer.find("Content-Length:");
	if (pos == std::string::npos) return 0; // No Content-Length header, assume no body

	pos += 15; // Move past "Content-Length:"
	while (pos < buffer.size() && isspace(buffer[pos])) ++pos; // Skip whitespace

	size_t endPos = buffer.find("\r\n", pos);
	if (endPos == std::string::npos) return -1; // Malformed header

	std::string lengthStr = buffer.substr(pos, endPos - pos);
	char* end;
	long value = std::strtol(lengthStr.c_str(), &end, 10);
	if (*end != '\0')
		return -1; // Invalid number
	return value;
}
