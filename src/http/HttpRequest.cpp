#include "HttpRequest.hpp"

HttpRequest::HttpRequest() {}
HttpRequest::~HttpRequest() {}

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
		stateMachine.state = READING_BODY;
	} else {
		stateMachine.state = DONE;
	}
}

void HttpRequest::processClient(ClientData &client) {
	StateMachine &stateMachine = client.stateMachine;
	std::string &buffer = stateMachine.buffer;
	
	switch (stateMachine.state) {
		case READING_REQUEST_LINE:
			try {
				HttpPart<RequestLine> part = Http::_parseRequestLine(buffer);

				// If size is 0, it means we don't have a complete request line yet, so we wait for more data
				if (part.size == 0) return;

				stateMachine.httpData.requestLine = part.value;
				stateMachine.state = READING_HEADERS;
				buffer.erase(0, part.size); // Remove the processed part from the buffer

			} catch (HttpException &e) {
				e._thrown = true;			// Mark the exception as thrown
				client.exception = e;		// Store the exception in the client data
				stateMachine.state = ERROR;	// Transition to error state
				return;
			}
			break;
		case READING_HEADERS:
			try {
				HttpPart<HeaderMap> part = Http::_parseHeaders(buffer);

				// If size is 0, it means we don't have complete headers yet, so we wait for more data
				if (part.size == 0) return;
				Http::_validateRequestLine(stateMachine.httpData.requestLine);
				Http::_validateHeaders(part.value);
				stateMachine.httpData.headers = part.value;
				fillClientData(client);
				
				buffer.erase(0, part.size); // Remove the processed part from the buffer

			} catch (HttpException &e) {
				e._thrown = true;			// Mark the exception as thrown
				client.exception = e;		// Store the exception in the client data
				stateMachine.state = ERROR;	// Transition to error state
				return;
			}
			break;
		case READING_BODY: 
			try {
				stateMachine.state = DONE;
			} catch (HttpException &e) {
				e._thrown = true;			// Mark the exception as thrown
				client.exception = e;		// Store the exception in the client data
				stateMachine.state = ERROR;	// Transition to error state
				return;
			}
			break;
		default:
			break;
	}
}