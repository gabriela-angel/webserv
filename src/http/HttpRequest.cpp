#include "HttpRequest.hpp"

HttpRequest::HttpRequest() {}
HttpRequest::~HttpRequest() {}

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


				if (part.value.count("Content-Length") > 0 || part.value.count("Transfer-Encoding") > 0) {
					stateMachine.state = READING_BODY;
				} else {
					stateMachine.state = DONE;
				}
				
				buffer.erase(0, part.size); // Remove the processed part from the buffer

			} catch (HttpException &e) {
				e._thrown = true;			// Mark the exception as thrown
				client.exception = e;		// Store the exception in the client data
				stateMachine.state = ERROR;	// Transition to error state
				return;
			}
			break;
		case READING_BODY: 
			(void)client; // To avoid unused parameter warning, we will implement body reading later
			break;
		default:
			break;
	}
}