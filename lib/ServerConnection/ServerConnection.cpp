#include "ServerConnection.h"

ServerConnection::ServerConnection() : http() {
}

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

uint8_t ServerConnection::reconnect() {
    http.end();

    return connect(url);
}

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

uint8_t ServerConnection::sendRequest(const char* request, int requestLen, const char* payload, int payloadLen, int* responseCode) {
    String requestString, payloadString, fullRequest;

    if (request == nullptr) {
        return ERROR_INVALID_REQUEST;
    }

    if (requestLen != REQUEST_SIZE) {
        return ERROR_INVALID_REQUEST;
    }

    requestString = charToString(request, requestLen);
    

    if (payload == nullptr) {
        return ERROR_INVALID_SERVER_PAYLOAD;
    }

    if (payloadLen != PAYLOAD_SIZE) {
        return ERROR_INVALID_SERVER_PAYLOAD;
    }

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

String ServerConnection::getResponse() {
    String response = http.getString();
    response.trim();
    return response;
}

String ServerConnection::getErrorToString(int httpResponseCode) {
    return http.errorToString(httpResponseCode).c_str();
}