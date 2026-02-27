#include "HttpRequest.hpp"

HttpRequest::HttpRequest() {}
HttpRequest::~HttpRequest() {}
Logger &HttpRequest::_logger = Logger::getInstance();

inline static void fillClientData(ClientData &client) {
	Http::HeaderMap &headers = client.stateMachine.httpData.headers;
	StateMachine &stateMachine = client.stateMachine;
	HttpData &httpData = stateMachine.httpData;

	httpData.host = headers[HOST][0];
	httpData.chunkedTransferEncoding = headers.count(TRANSFER_ENCODING) > 0;
	httpData.contentLength = headers.count(CONTENT_LENGTH) > 0 ? from_string<long long>(headers[CONTENT_LENGTH][0]) : 0;
	httpData.keepAlive = headers.count(CONNECTION) > 0 && headers[CONNECTION][0] == "keep-alive";
	httpData.expectContinue = headers.count(EXPECT) > 0;

	if (headers.count(CONTENT_LENGTH) > 0 || headers.count(TRANSFER_ENCODING) > 0) {
		stateMachine.state = StateMachine::READING_BODY;
	} else {
		stateMachine.state = StateMachine::DONE;
	}
}

void HttpRequest::processClient(ClientData &client) {
    StateMachine &sm = client.stateMachine;

    while (sm.state != StateMachine::DONE && sm.state != StateMachine::ERROR) {
        if (processClientState(client))
			break; // if need more data
    }
}

// This function will return true if we need to wait for more data
bool HttpRequest::processClientState(ClientData &client) {
	StateMachine &stateMachine = client.stateMachine;
	std::string &buffer = stateMachine.buffer;

	switch (stateMachine.state) {
		case StateMachine::READING_REQUEST_LINE:
			try {
				_logger.logDebug("Client " + to_string(client.clientSocket) + " State: READING_REQUEST_LINE");
				HttpPart<RequestLine> part = Http::_parseRequestLine(buffer);

				// If size is 0, it means we don't have a complete request line yet, so we wait for more data
				if (part.size == 0) return true;

				stateMachine.httpData.requestLine = part.value;
				stateMachine.state = StateMachine::READING_HEADERS;
				buffer.erase(0, part.size); // Remove the processed part from the buffer

			} catch (HttpException &e) {
				e._thrown = true;			// Mark the exception as thrown
				client.exception = e;		// Store the exception in the client data
				stateMachine.state = StateMachine::ERROR;	// Transition to error state
			}
			break;
		case StateMachine::READING_HEADERS:
			try {
				_logger.logDebug("Client " + to_string(client.clientSocket) + " State: READING_HEADERS");
				HttpPart<HeaderMap> part = Http::_parseHeaders(buffer);

				// If size is 0, it means we don't have complete headers yet, so we wait for more data
				if (part.size == 0) return true;
				Http::_validateRequestLine(stateMachine.httpData.requestLine);
				Http::_validateHeaders(part.value);
				stateMachine.httpData.headers = part.value;
				fillClientData(client);
				
				buffer.erase(0, part.size); // Remove the processed part from the buffer

			} catch (HttpException &e) {
				e._thrown = true;			// Mark the exception as thrown
				client.exception = e;		// Store the exception in the client data
				stateMachine.state = StateMachine::ERROR;	// Transition to error state
			}
			break;
		case StateMachine::READING_BODY: 
			try {
				_logger.logDebug("Client " + to_string(client.clientSocket) + " State: READING_BODY");
				/* 
					_parseBody() will parse and validate buffer with headers info.
					
					If TRANSFER-ENCODING: chunked,
					it will update client activity time to prevent timeout while waiting for the complete body.
				*/
				HttpPart<std::string> part = Http::_parseBody(stateMachine);
				if (part.size == 0) return true; // We don't have the complete body yet, wait for more data

				stateMachine.httpData.body = part.value;
				stateMachine.state = StateMachine::DONE;
				buffer.erase(0, part.size); // Remove the processed part from the buffer

			} catch (HttpException &e) {
				e._thrown = true;			// Mark the exception as thrown
				client.exception = e;		// Store the exception in the client data
				stateMachine.state = StateMachine::ERROR;	// Transition to error state
			}
			break;
		default:
			return false; // Should not happen, but just in case
			break;
	}
	return false;
}
