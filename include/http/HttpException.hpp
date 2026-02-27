#include <stdexcept>
#include "HttpUtils.hpp"

class HttpException : public std::exception {
	public:
		struct ParseError {
			enum Type
			{
				NONE = 0,
				
				// Semantic (can keep-alive)
				INVALID_URI,
				METHOD_NOT_ALLOWED,
				NOT_IMPLEMENTED,
				URI_TOO_LONG,
				PAYLOAD_TOO_LARGE,
				HTTP_VERSION_NOT_SUPPORTED,

				// Structural (MUST CLOSE)
				INVALID_START_LINE,
				MISSING_HOST_HEADER,
				MALFORMED_HEADER,
				INVALID_CONTENT_LENGTH,
				INVALID_TRANSFER_ENCODING,
				INVALID_CHUNK,
				HEADERS_TOO_LARGE,
				LENGTH_REQUIRED,
				UNSUPPORTED_MEDIA_TYPE,
				RANGE_NOT_SATISFIABLE
				
			};
		};


	private:
		HttpStatus::Code	_statusCode;
		ParseError::Type	_severity;
		bool 				_shouldClose;
		std::string 		_message;

		inline static HttpStatus::Code parseErrorToStatusCode(ParseError::Type severity);
			

	public:
		HttpException(void);
		HttpException(ParseError::Type severity);
		HttpException(HttpStatus::Code statusCode);
		~HttpException(void) throw(){};

		// Override
		const char* what() const throw() {
			return _message.c_str();
		}

		// Getters
		const HttpStatus::Code& getStatusCode() const {
			return _statusCode;
		}
		ParseError::Type getSeverity() const {
			return _severity;
		}
		bool shouldClose() const {
			return _shouldClose;
		}

		operator bool() const {
			return _severity != ParseError::NONE;
		}
};

#include "utils.tpp"