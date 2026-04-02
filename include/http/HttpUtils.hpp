#ifndef HTTPUTILS_HPP
# define HTTPUTILS_HPP
 
 
# include <cstddef>
# define HTTP_VERSION "HTTP/1.1"
# define CRLF "\r\n"
# define HEADER_END CRLF CRLF
 
struct HttpStatus
{
	enum Code
	{
		// =========================
		// 2xx Success
		// =========================
		OK = 200,
		CREATED = 201,
		NO_CONTENT = 204,
 
		// =========================
		// 3xx Redirection
		// =========================
		MOVED_PERMANENTLY = 301,
		FOUND = 302,
		SEE_OTHER = 303,
		TEMPORARY_REDIRECT = 307,
		PERMANENT_REDIRECT = 308,
 
		// =========================
		// 4xx Client Errors
		// =========================
		BAD_REQUEST = 400,
		FORBIDDEN = 403,
		NOT_FOUND = 404,
		METHOD_NOT_ALLOWED = 405,
		REQUEST_TIMEOUT = 408,
		CONFLICT = 409,
		LENGTH_REQUIRED = 411,
		CONTENT_TOO_LARGE = 413,
		URI_TOO_LONG = 414,
		UNSUPPORTED_MEDIA_TYPE = 415,
		RANGE_NOT_SATISFIABLE = 416,
 
		// =========================
		// 5xx Server Errors
		// =========================
		INTERNAL_SERVER_ERROR = 500,
		NOT_IMPLEMENTED = 501,
		BAD_GATEWAY = 502,
		SERVICE_UNAVAILABLE = 503,
		GATEWAY_TIMEOUT = 504,
		HTTP_VERSION_NOT_SUPPORTED = 505
	};
 
	inline static const char* reasonPhrase(HttpStatus::Code status)
	{
		switch(status)
		{
			// 2xx
			case HttpStatus::OK: return "OK";
			case HttpStatus::CREATED: return "Created";
			case HttpStatus::NO_CONTENT: return "No Content";
 
			// 3xx
			case HttpStatus::MOVED_PERMANENTLY: return "Moved Permanently";
			case HttpStatus::FOUND: return "Found";
			case HttpStatus::SEE_OTHER: return "See Other";
			case HttpStatus::TEMPORARY_REDIRECT: return "Temporary Redirect";
			case HttpStatus::PERMANENT_REDIRECT: return "Permanent Redirect";
 
			// 4xx
			case HttpStatus::BAD_REQUEST: return "Bad Request";
			case HttpStatus::FORBIDDEN: return "Forbidden";
			case HttpStatus::NOT_FOUND: return "Not Found";
			case HttpStatus::METHOD_NOT_ALLOWED: return "Method Not Allowed";
			case HttpStatus::CONFLICT: return "Conflict";
			case HttpStatus::REQUEST_TIMEOUT: return "Request Timeout";
			case HttpStatus::LENGTH_REQUIRED: return "Length Required";
			case HttpStatus::CONTENT_TOO_LARGE: return "Content Too Large";
			case HttpStatus::URI_TOO_LONG: return "URI Too Long";
			case HttpStatus::UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";
			case HttpStatus::RANGE_NOT_SATISFIABLE: return "Range Not Satisfiable";
			// 5xx
			case HttpStatus::INTERNAL_SERVER_ERROR: return "Internal Server Error";
			case HttpStatus::NOT_IMPLEMENTED: return "Not Implemented";
			case HttpStatus::BAD_GATEWAY: return "Bad Gateway";
			case HttpStatus::SERVICE_UNAVAILABLE: return "Service Unavailable";
			case HttpStatus::GATEWAY_TIMEOUT: return "Gateway Timeout";
			case HttpStatus::HTTP_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";
 
			default: return "Unknown Status";
		}
	}
};
 
 
struct Content {
	enum Type {
 
		// Text types
		TEXT_PLAIN,
		TEXT_HTML,
		TEXT_CSS,
		TEXT_JAVASCRIPT,
		TEXT_CSV,
		TEXT_XML,
 
		// Application types
		APPLICATION_JSON,
		APPLICATION_XML,
		APPLICATION_PDF,
		APPLICATION_ZIP,
		APPLICATION_GZIP,
		APPLICATION_OCTET_STREAM,
		APLICATION_FORM_URLENCODED,
		APLICATION_JAVASCRIPT,
		APLICATION_SQL,
		APLICATION_WASM,
 
		// Multipart types
		MULTIPART_FORM_DATA,
		MULTIPART_MIXED,
		MULTPART_BYTERANGES,
 
		// Image types
		IMAGE_PNG,
		IMAGE_JPEG,
		IMAGE_GIF,
		IMAGE_WEBP,
		IMAGE_SVG_XML,
		IMAGE_BMP,
		IMAGE_ICON,
 
		// Audio types
		AUDIO_MPEG,
		AUDIO_WAV,
		AUDIO_OGG,
		AUDIO_WEBM,
 
		// Video types
		VIDEO_MP4,
		VIDEO_WEBM,
		VIDEO_OGG,
		VIDEO_MSVIDEO,
 
		// Font types
		FONT_WOFF,
		FONT_WOFF2,
		FONT_TTF,
		FONT_OTF,
 
	};
 
};
 
template <typename T>
struct HttpPart
{
	T value;
	size_t size;
 
	HttpPart() : value(), size(0) {}
 
};
 
#endif