#include <Arduino.h>
#include <WiFi.h>

#include <time.h>

#include <ScreenOutput.h>
#include <ServerConnection.h>

#include <led.h>
#include <button.h>

#include "Identifier.h"

/*
==================================================================================================
                                        CONFIGURATION
==================================================================================================
*/

#define GREEN_LED_PIN 9
#define ORANGE_LED_PIN 0
#define RED_LED_PIN 1

#define LEFT_BUTTON_PIN 18
#define MIDDLE_BUTTON_PIN 5

#define RESERVATION_TIME 15 // in minutes

#define DELAY_BETWEEN_CHECKS_CONNECTION 30000 // in milliseconds
#define DELAY_BETWEEN_CHECKS_NO_RESERVATION 15000 // in milliseconds
#define DELAY_BETWEEN_CHECKS_HOURS 15000 // in milliseconds

#define TIME_BEFORE_SHOWING_RESERVATION 180

#define RESERVATION_CANCELLED "4" // Reservation state for cancelled in the sql enum
#define RESERVATION_ONGOING "1" // Reservation state for ongoing in the sql enum

const char* deviceId = "00000016";  // Id of the device registered on the server

const long gmtOffset_sec = 3600;   // GMT offset in seconds

/*
==================================================================================================
                                        SCRIPT
==================================================================================================
*/

extern "C" int lwip_hook_ip6_input(void *p) {
  return 1; // Retourne 1 pour indiquer que le paquet IPv6 est accepté
}

ServerConnection server;

ScreenOutput screenOutput;

Led greenLed = Led(GREEN_LED_PIN);
Led orangeLed = Led(ORANGE_LED_PIN);
Led redLed = Led(RED_LED_PIN);

Button leftButton = Button(LEFT_BUTTON_PIN);
Button middleButton = Button(MIDDLE_BUTTON_PIN);

const char* ntpServer = "pool.ntp.org";    // NTP server 
const int daylightOffset_sec = 3600;      // Daylight saving time offset in seconds

bool checkWifiConnection();

void mainScreen();

int checkReservations();

void changeReservationState(String reservationID, String newState);

void getReservationName(String reservationID, String& name, String& surname);

void getTime(int* hours, int* minutes);

String formatTime(int time);

bool isConnected = false;

void setup() {
  int currentHour, currentMinute;

  Serial.println("Starting ResaLab...");

  screenOutput.begin();
  WiFi.begin(ssid, password);
  server.connect(serverUrl);
  server.checkConnection();


  while (!isConnected) {
    isConnected = checkWifiConnection();
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  screenOutput.resetScreen();

  getTime(&currentHour, &currentMinute);

  screenOutput.showHour(currentHour, currentMinute);
  screenOutput.showInternetState(true, WiFi.RSSI());
}

uint64_t lastTimeConnectionCheck = 0, lastTimeReservationCheck = 0, lastTimeScreenUpdated = 0;

bool isToValid = true;
int currentReservationId = -1;

void loop() {

  if (millis() - lastTimeConnectionCheck > DELAY_BETWEEN_CHECKS_CONNECTION || !isConnected) {
    lastTimeConnectionCheck = millis();
    isConnected = checkWifiConnection();
  }

  if (!isConnected) {
    screenOutput.showInternetState(false);
    return;
  }

  if (millis() - lastTimeScreenUpdated > DELAY_BETWEEN_CHECKS_HOURS) {
    lastTimeScreenUpdated = millis();

    int currentHour, currentMinute;
    getTime(&currentHour, &currentMinute);

    screenOutput.showHour(currentHour, currentMinute);
    screenOutput.showInternetState(true, WiFi.RSSI());
  }

  if (millis() - lastTimeReservationCheck > DELAY_BETWEEN_CHECKS_NO_RESERVATION) {
    lastTimeReservationCheck = millis();
    log_v("Checking reservations...");
    currentReservationId = checkReservations();
  }

  if (currentReservationId == -2) {
    screenOutput.showText("Erreur de connexion au serveur");
    return;
  }

  if (leftButton.isPressed() && isToValid && currentReservationId != -1) {
    changeReservationState((String)currentReservationId, RESERVATION_ONGOING);
    isToValid = false;
  }

  if (middleButton.isPressed() && currentReservationId != -1) {
    changeReservationState((String)currentReservationId, RESERVATION_CANCELLED);
    isToValid = false;
    redLed.off();
    greenLed.on();
    orangeLed.off();
  }
}

int checkReservations() {
  int httpResponseCode = 0;

  int errorCode = server.sendRequest(GET_NEXT_RESERVATION, strlen(GET_NEXT_RESERVATION), deviceId, strlen(deviceId), &httpResponseCode);
  String response = server.getResponse();

  if (errorCode != CODE_SUCCESS) {
    log_e("Failed to send get next reservation request. Error code: %d", errorCode);
    return -1;
  }

  log_d("Server response: %s", response.c_str());

  String serverResponseCode = response.substring(0, 3);

  if (serverResponseCode.equals(INVALID_RESERVATION)) {
    log_i("No reservation found.");
    greenLed.on();
    redLed.off();
    orangeLed.off();
    screenOutput.showText("Aucune reservation");
    return -1;
  }

  int reservationId = response.substring(3, 8).toInt();
  bool reservationComing = response.substring(8, 9).toInt();
  int startReservationTimestamp = response.substring(9, 19).toInt();
  int endReservationTimestamp = response.substring(19, 29).toInt();

  log_v("Server response code: %s", serverResponseCode.c_str());
  log_v("reservationComing: %d", reservationComing);
  log_v("reservationId: %d", reservationId);
  log_v("startReservationTimestamp: %d", startReservationTimestamp);
  log_v("endReservationTimestamp: %d", endReservationTimestamp);

  time_t now = time(nullptr);
  if (now == -1) {
    log_e("Failed to obtain timestamp");
    return -1;
  }

  log_v("Current timestamp: %ld", now);

  if (!reservationComing && now > startReservationTimestamp && now < endReservationTimestamp) {
    redLed.on();
    greenLed.off();
    orangeLed.off();

    String name = "";
    String surname = "";
    getReservationName((String)currentReservationId, name, surname);
    getReservationName(String(reservationId), name, surname);
    screenOutput.showText("Reservation en cours : ", name + " " + surname);

    int progression = (int)(((float)(now - startReservationTimestamp) / (float)(endReservationTimestamp - startReservationTimestamp)) * 100);
    screenOutput.showProgressionBar(progression);
    
  } else if ( now > startReservationTimestamp && now < startReservationTimestamp + RESERVATION_TIME * 60) {
    greenLed.off();
    redLed.off();
    orangeLed.on();
    isToValid = true;
    screenOutput.showText("Validez la reservation");
  } else {
    redLed.off();
    greenLed.on();
    orangeLed.off();
    if ( now + TIME_BEFORE_SHOWING_RESERVATION * 60 > startReservationTimestamp) {
      // Adding 60 seconds to round up.
      int timeBeforeReservation = startReservationTimestamp - now + 60;

      screenOutput.showText("Prochaine reservation dans", formatTime(timeBeforeReservation));
    } else {
      screenOutput.showText("Machine Libre");
    }
  }
  

  if (reservationComing && now > startReservationTimestamp + RESERVATION_TIME * 60) {
    changeReservationState(String(reservationId), RESERVATION_CANCELLED);
    isToValid = false;
    redLed.off();
    greenLed.on();
    orangeLed.off();
  }

  return reservationId;
}

void changeReservationState(String reservationID, String newState) {
  int httpResponseCode = 0;

  log_d("ID : %s, new state : %s", reservationID.c_str(), newState.c_str());

  String payload = "";
  for (int i = reservationID.length(); i < 5; i++) {
    payload += "0";
  }
  payload += reservationID;
  for (int i = newState.length(); i < 3; i++) {
    payload += "0";
  }
  payload += newState;

  int errorCode = server.sendRequest(CHANGE_RESERVATION_STATE, strlen(CHANGE_RESERVATION_STATE), payload.c_str(), payload.length(), &httpResponseCode);
  String response = server.getResponse();

  if (errorCode != CODE_SUCCESS) {
    log_e("Failed to send change reservation state request. Error code: %d", errorCode);
    return;
  }
  log_d("Server response: %s", response.c_str());

  String serverResponseCode = response.substring(0, 3);

  if (serverResponseCode.equals(RESERVATION_STATE_CHANGED)) {
    log_i("Reservation state changed successfully.");
    return;
  } else {
    log_e("Failed to change reservation state.");
    return;
  }
}

bool checkWifiConnection() {
  uint8_t errorCode;

  errorCode = server.checkConnection();

  if (errorCode == CODE_SUCCESS) {
    return true;
  }

  log_d("Connection lost. Error code: %d", errorCode);

  WiFi.reconnect();
  errorCode = server.reconnect();

  if (errorCode == ERROR_WIFI_DISCONNECTED) {
    log_v("Wifi disconnected");
    orangeLed.blink(1000);
    return false;
  }

  if (errorCode == ERROR_SERVER_DISCONNECTED) {
    log_v("Server disconnected");
    orangeLed.blink(250, 1000);
    return false;
  }

  return true;
}

void getReservationName(String reservationID, String& name, String& surname) {
  int httpResponseCode = 0;

  // Pad reservation ID with leading zeros to ensure it is 8 characters long
  String payload = "";
  for (int i = reservationID.length(); i < 8; i++) {
    payload += "0";
  }
  payload += reservationID;

  int errorCode = server.sendRequest(GET_RESERVATION_NAME, strlen(GET_RESERVATION_NAME), payload.c_str(), payload.length(), &httpResponseCode);
  String response = server.getResponse();

  if (errorCode != CODE_SUCCESS) {
    log_e("Failed to get reservation name. Error code: %d", errorCode);
    name = String(errorCode);
    surname = "";
    return;
  }

  log_d("Server response: %s", response.c_str());

  String serverResponseCode = response.substring(0, 3);

  if (serverResponseCode.equals(INVALID_RESERVATION)) {
    log_e("Invalid reservation ID.");
    name = serverResponseCode;
    surname = "";
    return;
  }

  if (serverResponseCode.equals(USER_NOT_FOUND)) {
    log_e("User not found for reservation ID: %s", reservationID.c_str());
    name = serverResponseCode;
    surname = "";
    return;
  }

  // Remove the first 3 characters which represent the response code
  response = response.substring(3); // Remove response code

  // Split the response into name and surname using '&' as the delimiter
  // Example response: "Mouchere&Ludovic"
  int sep = response.indexOf('&');
  if (sep < 0) {
    name = response;
    surname = "";
    return;
  }

  name = response.substring(0, sep);
  surname = response.substring(sep + 1);
}

/**
 * @brief Retrieves the current local time and stores the hours and minutes.
 * 
 * @param[out] hours Pointer to an integer where the current hour (0-23) will be stored.
 * @param[out] minutes Pointer to an integer where the current minute (0-59) will be stored.
 * 
 */
void getTime(int* hours, int* minutes) {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  *hours = timeinfo->tm_hour;
  *minutes = timeinfo->tm_min;
}

/**
 * @brief Formats a time value in seconds into a human-readable string format.
 * 
 * @param time The time value in seconds to be formatted.
 * @return String A formatted time string in the format "Xh Ym" (if hours > 0) 
 *         or "Ym" (if hours == 0), where X is hours and Y is minutes.
 * 
 * @note If the input time is less than 60 seconds, it will be displayed as "0m".
 * 
 */
String formatTime(int time) {
  int minutes, hours;

  minutes = time / 60;
  hours = minutes / 60;
  minutes = minutes % 60;

  String timeFormatted = "";

  if (hours > 0) {
    timeFormatted += String(hours) + "h";
  }
  timeFormatted += String(minutes) + "m" ;

  return timeFormatted;
}
