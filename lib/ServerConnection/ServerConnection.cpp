#include "ServerConnection.h"

ServerConnection::ServerConnection() : http() {
}

/**
 * @brief Establishes a connection to the specified server URL.
 * 
 * @param url The server URL to connect to as a String.
 * 
 * @return uint8_t Status code indicating the result of the connection attempt.
 * 
 */
uint8_t ServerConnection::connect(String url) {
    bool success;

    this->url = url;

    success = http.begin(url);
    if (!success) {
        return ERROR_SERVER_CONNECTION_FAILED;
    }

    http.addHeader("Content-Type", "text/plain");

    errorCode = checkConnection();
    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    return CODE_SUCCESS;
}

/**
 * @brief Reconnects to the server by closing the current connection and establishing a new one.
 * 
 * @return uint8_t Status code indicating the success or failure of the reconnection attempt.
 * 
 */
uint8_t ServerConnection::reconnect() {
    http.end();

    return connect(url);
}

/**
 * @brief Checks the connection status with the server.
 * 
 * This function verifies both the WiFi connection and the server availability. It verify if the reservation
 * service is reachable by sending a test request. A normal server would not Respond to this request with "CONNECTION_OK".
 * 
 * @return uint8_t Status code indicating the connection state.
 */
uint8_t ServerConnection::checkConnection() {
    int httpResponseCode;

    // Check wifi connection
    if (WiFi.status() != WL_CONNECTED) {
        log_e("Wifi disconnected. %d", WiFi.status());
        return ERROR_WIFI_DISCONNECTED;
    }

    if (url.isEmpty()) {
        log_e("URL is empty. Cannot check connection.");
        return ERROR_SERVER_CONNECTION_FAILED;
    }

    // Check if the server responds
    errorCode = sendRequest(TEST_CONNECTION, REQUEST_SIZE, "00000000", PAYLOAD_SIZE, &httpResponseCode);

    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    String serverResponse = getResponse();

    if (!serverResponse.equals(CONNECTION_OK)) {
        log_e("Invalid server response: %s", serverResponse.c_str());
        return ERROR_INVALID_SERVER_RESPONSE;
    }

    return CODE_SUCCESS;
}

/**
 * @brief Sends an HTTP POST request to the server with the specified request and payload.
 * 
 * @param request Pointer to the request character array. Must not be nullptr.
 * @param requestLen Length of the request array. Must equal REQUEST_SIZE.
 * @param payload Pointer to the payload character array. Must not be nullptr.
 * @param payloadLen Length of the payload array. Must equal PAYLOAD_SIZE.
 * @param responseCode Pointer to an integer where the HTTP response code will be stored.
 * 
 * @return uint8_t Status code indicating the result of the operation.
 */
uint8_t ServerConnection::sendRequest(const char* request, int requestLen, const char* payload, int payloadLen, int* responseCode) {
    String requestString, payloadString, fullRequest;

    if (request == nullptr || requestLen != REQUEST_SIZE) {
        return ERROR_INVALID_REQUEST;
    }

    if (payload == nullptr || payloadLen != PAYLOAD_SIZE) {
        return ERROR_INVALID_SERVER_PAYLOAD;
    }

    requestString = charToString(request, requestLen);
    payloadString = charToString(payload, payloadLen);

    fullRequest = requestString + payloadString;
    log_d("Full request: %s", fullRequest.c_str());

    *responseCode = http.POST(fullRequest);

    if (*responseCode <= 0) {
        log_e("HTTP POST failed. Error: %s", http.errorToString(*responseCode).c_str());
        return ERROR_SERVER_DISCONNECTED;
    }
    
    log_d("HTTP POST Response Code: %d", *responseCode);

    return CODE_SUCCESS;
}

/**
 * @brief Retrieves and formats the HTTP response string from the server.
 * 
 * @return String The trimmed HTTP response string from the server.
 * 
 */
String ServerConnection::getResponse() {
    String response = http.getString();
    response.trim();
    return response;
}

/**
 * @brief Converts an HTTP response code to its corresponding error message string.
 * 
 * @param httpResponseCode The HTTP response code to be converted to a string.
 * 
 * @return String A descriptive error message corresponding to the HTTP response code.
 *               Returns an empty string or generic message if the code is not recognized.
 */
String ServerConnection::getErrorToString(int httpResponseCode) {
    return http.errorToString(httpResponseCode).c_str();
}