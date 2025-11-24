#ifndef SERVER_REQUEST_H
#define SERVER_REQUEST_H

#include <Arduino.h>

#define REQUEST_SIZE 3 // Size of the request string

// Server response
constexpr const char* VALID_CARD_ID = "001";          // 001 = The card is in the database and is valid
constexpr const char* INVALID_CARD_ID = "002";        // 002 = The card is either not in the database or is invalid
constexpr const char* INVALID_FORMAT = "003";         // 003 = The card ID is not in the correct format
constexpr const char* VALID_RESERVATION = "004";       // 004 = There is a reservation
constexpr const char* INVALID_RESERVATION = "005";     // 005 = There is no reservation
constexpr const char* RESERVATION_STATE_CHANGED = "006"; // 006 = The reservation state has been changed successfully
constexpr const char* RESERVATION_STATE_NOT_CHANGED = "007"; // 007 = The reservation state has not been changed
constexpr const char* USER_FOUND = "008";               // 008 = User found
constexpr const char* USER_NOT_FOUND = "009";           // 009 = User not found
constexpr const char* CONNECTION_OK = "200";         // 200 = Connection to the server is OK
constexpr const char* INVALID_REQUEST = "400";        // 400 = The request is invalid

// Server request
constexpr const char* TEST_CONNECTION = "100";          // 100 = Test connection
constexpr const char* CARD_ID_VERIFICATION = "101";   // 101 = Card ID verification
constexpr const char* GET_NEXT_RESERVATION = "102";   // 102 = Get the next reservation
constexpr const char* CHANGE_RESERVATION_STATE = "103"; // 103 = Change reservation state
constexpr const char* GET_RESERVATION_NAME = "104"; // 104 = Get reservation name

#endif // SERVER_REQUEST_H
