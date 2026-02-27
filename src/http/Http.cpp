#include "Http.hpp"

static inline bool isSpace(char c) {
	return c == ' ' || c == '\t';
}

static inline bool isValidTokenChar(char c) {
	return std::isalnum(c) || c == '-' || c == '_';
}

static inline bool isUnreserved(char c) {
	return std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~';
}

static inline bool isReserved(char c) {
	return c == ':' || c == '/' || c == '?' || c == '#' || c == '[' || c == ']' ||
		   c == '@' || c == '!' || c == '$' || c == '&' || c == '\'' || 
		   c == '(' || c == ')' || c == '*' || c == '+' || c == ',' || 
		   c == ';' || c == '=';
}

HttpPart<RequestLine> Http::_parseRequestLine(const std::string &buffer)
{
	if (buffer.size() > MAX_REQUEST_LINE_SIZE)
		throw HttpException(HttpException::ParseError::INVALID_START_LINE);

	size_t pos = buffer.find(CRLF);
	if (pos == std::string::npos) return HttpPart<RequestLine>();

	std::string line = buffer.substr(0, pos);
	
	RequestLine				requestLine;
	std::istringstream		iss(line);
	HttpPart<RequestLine>	part;

	size_t spaceCount = std::count(line.begin(), line.end(), ' ');
	if (spaceCount != 2)
		throw HttpException(HttpException::ParseError::INVALID_START_LINE);
	
	if (!(iss >> requestLine.method >> requestLine.uri >> requestLine.version))
		throw HttpException(HttpException::ParseError::INVALID_START_LINE);
	
	part.value = requestLine;
	part.size = pos + 2; // Include the size of CRLF
	return part;
}

void Http::_validateRequestLine(const RequestLine &requestLine)
{
	std::string arr[] = HTTP_METHODS;
	std::vector<std::string> validMethods(arr, arr + sizeof(arr) / sizeof(arr[0]));

	std::string method = requestLine.method;
	std::string uri = requestLine.uri;
	std::string version = requestLine.version;

	if (std::find(validMethods.begin(), validMethods.end(), method) == validMethods.end())
		throw HttpException(HttpException::ParseError::METHOD_NOT_ALLOWED);
	
	if (uri[0] != '/' && !(method == "OPTIONS" && uri == "*"))
			throw HttpException(HttpException::ParseError::INVALID_URI);

	if (uri.size() > MAX_URI_SIZE)
		throw HttpException(HttpException::ParseError::URI_TOO_LONG);

	for (size_t i = 0; i < uri.size(); ++i) {
		char c = uri[i];
		if (!isUnreserved(c) && !isReserved(c) && c != '%')
			throw HttpException(HttpException::ParseError::INVALID_URI);
	}

	std::string tmp(uri);
	size_t percentPos;
	while ((percentPos = tmp.find('%')) != std::string::npos) {
		if (percentPos + 2 >= tmp.size() ||
			!std::isxdigit(tmp[percentPos + 1]) ||
			!std::isxdigit(tmp[percentPos + 2])) {
			throw HttpException(HttpException::ParseError::INVALID_URI);
		}
		tmp = tmp.substr(percentPos + 3); // Move past the percent-encoded sequence
	}

	if (uri.find("//") != std::string::npos)
		throw HttpException(HttpException::ParseError::INVALID_URI);

	if (uri.find("..") != std::string::npos)
		throw HttpException(HttpException::ParseError::INVALID_URI);

	if (version != HTTP_VERSION)
		throw HttpException(HttpException::ParseError::HTTP_VERSION_NOT_SUPPORTED);
}

HttpPart<Http::HeaderMap> Http::_parseHeaders(const std::string &buffer)
{
	if (buffer.size() > MAX_HEADER_SIZE)
	throw HttpException(HttpException::ParseError::HEADERS_TOO_LARGE);
	
	// Check if we have the end of headers
	size_t end = buffer.find(HEADER_END);
	if (end == std::string::npos) return HttpPart<Http::HeaderMap>();

	Http::HeaderMap headers;
	size_t start = 0;

	size_t headerCount = 0;
	while (start < end) {
		size_t lineEnd = buffer.find(CRLF, start);
		if (lineEnd == std::string::npos || lineEnd > end)
			throw HttpException(HttpException::ParseError::MALFORMED_HEADER);

		std::string line = buffer.substr(start, lineEnd - start);
		if (line.empty()) throw HttpException(HttpException::ParseError::MALFORMED_HEADER);

		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
			throw HttpException(HttpException::ParseError::MALFORMED_HEADER);


		// Ensure there's no space before the colon
		if (colonPos == 0 || isSpace(line[colonPos - 1]))
			throw HttpException(HttpException::ParseError::MALFORMED_HEADER);



		std::string name = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);


		for (size_t i = 0; i < name.size(); ++i)
        {
            unsigned char c = name[i];
            if (!isValidTokenChar(c))
                throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
			name[i] = std::toupper(c);
        }



		// Trim value Whitespace
		value = trim(value);

		for (size_t i = 0; i < value.size(); ++i) {
			unsigned char c = value[i];
			if (c == '\r' || c == '\n' || c == '\0')
				throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
			if (c < 0x20 && c != '\t')
				throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
		}

		headers[name].push_back(value);
		headerCount++;
		start = lineEnd + 2; // Move past CRLF
	}

	if (headerCount > MAX_HEADERS)
		throw HttpException(HttpException::ParseError::HEADERS_TOO_LARGE);

	HttpPart<Http::HeaderMap> part;
	part.value = headers;
	part.size = end + 4; // Include the size of the delimiter
	return part;
}

void Http::_validateHeaders(const Http::HeaderMap &headers)
{
	bool hasContentLength = false;
	bool hasTransferEncoding = false;
	bool hasHost = false;
	for (ConstHeaderIterator it = headers.begin(); it != headers.end(); ++it) {
		const HeaderKey &name = it->first;
		const HeaderValues &values = it->second;

	/* Validate CONTENT-LENGTH */
		if (name == CONTENT_LENGTH) {
			hasContentLength = true;
			std::string mainValue = values[0];

			// Content-Length cannot be empty
			if (mainValue.empty())
				throw HttpException(HttpException::ParseError::INVALID_CONTENT_LENGTH);

			// If we find a different value, it's a malformed header
			for (ConstHeaderValueIterator itValue = values.begin(); itValue != values.end(); ++itValue)
				if (*itValue != mainValue)
					throw HttpException(HttpException::ParseError::MALFORMED_HEADER);

			// Check if the value is a valid non-negative integer and within the allowed body size
			for (size_t i = 0; i < mainValue.size(); ++i)
				if (!std::isdigit(static_cast<unsigned char>(mainValue[i])))
					throw HttpException(HttpException::ParseError::INVALID_CONTENT_LENGTH);
			
			// Convert to long long and check range
			long long contentLength = from_string<long long>(mainValue);
			if (contentLength < 0 || contentLength > MAX_BODY_SIZE)
				throw HttpException(HttpException::ParseError::INVALID_CONTENT_LENGTH);

			// If Content-Length AND Transfer-Encoding is present, it's an error
			if (hasTransferEncoding)
				throw HttpException(HttpException::ParseError::INVALID_CONTENT_LENGTH);
		}

	/* Validate TRANSFER-ENCODING */
		if (name == TRANSFER_ENCODING) {
			hasTransferEncoding = true;

			// has only one value and it must be "chunked"
			if (values.size() != 1 || values[0] != "chunked")
				throw HttpException(HttpException::ParseError::INVALID_TRANSFER_ENCODING);			

			// If Transfer-Encoding AND Content-Length is present, it's an error
			if (hasContentLength)
				throw HttpException(HttpException::ParseError::INVALID_TRANSFER_ENCODING);
		}
	
	/* Validate HOST */
		if (name == HOST)
		{
			hasHost = true;
			if (values.size() != 1)
				throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
			std::string host = values[0];
			if (host.empty())
				throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
			
			// Validate host format
			size_t colonPos = host.find(':');
			std::string hostname = (colonPos == std::string::npos) ? host : host.substr(0, colonPos);
			std::string portStr = (colonPos == std::string::npos) ? "" : host.substr(colonPos + 1);

			// Validate hostname
			if (hostname.size() > MAX_HOST_SIZE)
				throw HttpException(HttpException::ParseError::MALFORMED_HEADER);

			typedef std::vector<std::string>	StringVector;
			typedef StringVector::iterator		StringVectorIterator;

			StringVector labels = split(hostname, '.');
			for (StringVectorIterator it = labels.begin(); it != labels.end(); ++it) {
				const std::string &label = *it;
				if (label.empty() || label.size() > MAX_HOST_LABEL_SIZE)
					throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
				if (!std::isalnum(static_cast<unsigned char>(label[0])) || !std::isalnum(static_cast<unsigned char>(label[label.size() - 1])))
					throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
				for (size_t i = 1; i < label.size() - 1; ++i) {
					char c = label[i];
					if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-')
						throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
				}
			}

			// Validate port if present
			if (!portStr.empty()) {
				if (portStr.size() > 5) // Max port number is 65535
					throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
				for (size_t i = 0; i < portStr.size(); ++i)
					if (!std::isdigit(static_cast<unsigned char>(portStr[i])))
						throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
				int port = from_string<int>(portStr);
				if (port < 1 || port > 65535)
					throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
			}
		}
	
	/* Validate CONNECTION */
		if (name == CONNECTION) {
			// If we have multiple Connection headers or multiple values, it's a malformed header
			if (values.size() > 1)
				throw HttpException(HttpException::ParseError::MALFORMED_HEADER);

			std::string value = values[0];
			if (value != "keep-alive" && value != "close")
				throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
		}
	
	/* Validate EXPECT */
		if (name == EXPECT) {
			// If we have multiple Expect headers or multiple values, it's a malformed header
			if (values.size() > 1 || values[0] != "100-continue")
				throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
			
			// If Expect is present without Content-Length or Transfer-Encoding, it's an LENGTH_REQUIRED error
			if (!hasContentLength && !hasTransferEncoding)
				throw HttpException(HttpException::ParseError::LENGTH_REQUIRED);
		}

	/* Validate CONTENT-TYPE */
		if (name == CONTENT_TYPE) {
			if (values.size() != 1)
				throw HttpException(HttpException::ParseError::MALFORMED_HEADER);

			// Ex: "text/html; charset=UTF-8; boundary=something"
			std::string value = values[0];
			
			
			std::vector<std::string> parts = split(value, ';');
			if (parts.empty())
				throw HttpException(HttpException::ParseError::UNSUPPORTED_MEDIA_TYPE);
			
			std::string mediaType = parts[0];
			std::vector<std::string> mediaTypeParts = split(mediaType, '/');
			if (mediaTypeParts.size() != 2)
				throw HttpException(HttpException::ParseError::UNSUPPORTED_MEDIA_TYPE);
			
			std::vector<std::string> parameters(parts.begin() + 1, parts.end());
			
			for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
				std::string param = trim(*it);
				size_t equalsPos = param.find('=');
				if (equalsPos == std::string::npos)
					throw HttpException(HttpException::ParseError::UNSUPPORTED_MEDIA_TYPE);
			}
		}

	/* Validate RANGE */
	// Range will be ignored for now.

	}
	if (!hasHost)
		throw HttpException(HttpException::ParseError::MISSING_HOST_HEADER);
}

HttpPart<std::string>	Http::_parseBody(StateMachine &stateMachine)
{
	const std::string	&buffer = stateMachine.buffer;
	HttpData			&httpData = stateMachine.httpData;

/* CONTENT-LENGTH */ //
	if (httpData.contentLength > 0) {
		if (buffer.size() < httpData.contentLength)
			return HttpPart<std::string>(); // Wait for more data (Have CLI_TIMEOUT time to receive the body, otherwise close the connection)

		HttpPart<std::string> part;
		part.value = buffer.substr(0, httpData.contentLength);
		part.size = httpData.contentLength;
		return part;
	}

/* TRANSFER-ENCODING: chunked */
	if (httpData.chunkedTransferEncoding) {
		if (buffer.size() > MAX_BODY_SIZE)
			throw HttpException(HttpException::ParseError::PAYLOAD_TOO_LARGE);
		
		// Implement a ChunkStateMachine to parse the chunked body
		// update client Activity
	}

	return HttpPart<std::string>(); // No body expected (connection will close in CLI_TIMEOUT seconds)
}