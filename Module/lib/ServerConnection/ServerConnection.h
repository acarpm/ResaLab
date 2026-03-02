#ifndef SERVER_CONNECTION_H
#define SERVER_CONNECTION_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "ServerRequest.h"

#define CODE_SUCCESS (0)



// Server error codes
#define ERROR_SERVER_CONNECTION_FAILED  (200) // Connection to the server failed
#define ERROR_INVALID_REQUEST           (201) // The request is invalid
#define ERROR_INVALID_SERVER_PAYLOAD    (202) // The server payload is invalid
#define ERROR_WIFI_DISCONNECTED         (203) // The wifi is disconnected
#define ERROR_SERVER_DISCONNECTED       (204) // The server is disconnected
#define ERROR_INVALID_SERVER_RESPONSE   (205) // The server response is invalid

#define PAYLOAD_SIZE 8 // Size of the payload string

class ServerConnection {

    private:
        HTTPClient http;

        uint8_t errorCode = CODE_SUCCESS;

        String url;

        String charToString(const char* str, int len) {
            String result = "";
            for (int i = 0; i < len; i++) {
                result += str[i];
            }
            return result;
        }

    public:
        ServerConnection(void);

        uint8_t connect(String url);
        uint8_t reconnect();
        uint8_t checkConnection(void);
        uint8_t sendRequest(const char* request, int requestLen, const char* payload, int payloadLen, int* responseCode);
        
        String getResponse(void);
        String getErrorToString(int httpResponseCode);
};

#endif // SERVER_CONNECTION_H