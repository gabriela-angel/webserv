#include "HttpException.hpp"


HttpException::HttpException() : 
	_statusCode(HttpStatus::BAD_REQUEST),
	_severity(ParseError::NONE),
	_shouldClose(false),
	_message("HTTP Error: Bad Request")
{}

HttpException::HttpException(HttpStatus::Code statusCode) : 
	_statusCode(statusCode),
	_severity(ParseError::NONE),
	_shouldClose(false)
{
	_message = "HTTP Error: " + to_string(statusCode) + " " + std::string(HttpStatus::reasonPhrase(statusCode));
}

HttpException::HttpException(ParseError::Type severity) :
	_severity(severity),
	_shouldClose(false)
{
	_statusCode = parseErrorToStatusCode(severity);
	if (severity >= ParseError::INVALID_START_LINE)
		_shouldClose = true;
	_message = "HTTP Parse Error: " + to_string(severity);
}

HttpStatus::Code HttpException::parseErrorToStatusCode(ParseError::Type severity) {
	switch (severity) {
		case ParseError::INVALID_URI:
		case ParseError::INVALID_CHUNK:
		case ParseError::MALFORMED_HEADER:
		case ParseError::HEADERS_TOO_LARGE:
		case ParseError::INVALID_START_LINE:
		case ParseError::MISSING_HOST_HEADER:
		case ParseError::INVALID_CONTENT_LENGTH:
		case ParseError::INVALID_TRANSFER_ENCODING:
			return HttpStatus::BAD_REQUEST;

		case ParseError::METHOD_NOT_ALLOWED:
			return HttpStatus::METHOD_NOT_ALLOWED;
		case ParseError::NOT_IMPLEMENTED:
			return HttpStatus::NOT_IMPLEMENTED;
		case ParseError::URI_TOO_LONG:
			return HttpStatus::URI_TOO_LONG;
		case ParseError::PAYLOAD_TOO_LARGE:
			return HttpStatus::CONTENT_TOO_LARGE;
		case ParseError::HTTP_VERSION_NOT_SUPPORTED:
			return HttpStatus::HTTP_VERSION_NOT_SUPPORTED;
		case ParseError::LENGTH_REQUIRED:
			return HttpStatus::LENGTH_REQUIRED;
		case ParseError::UNSUPPORTED_MEDIA_TYPE:
			return HttpStatus::UNSUPPORTED_MEDIA_TYPE;
		case ParseError::RANGE_NOT_SATISFIABLE:
			return HttpStatus::RANGE_NOT_SATISFIABLE;


		default:
			return HttpStatus::BAD_REQUEST;
	}
}