#include <Arduino.h>
#include <WiFi.h>

#include <time.h>

#include <ScreenOutput.h>
#include <ServerConnection.h>

#include <led.h>
#include <button.h>

#include "Identifier.h"

/*
TODO
RAJOUTER UN BOOKING ID AU REQUETE SERVEUR PLUS FINISH TIME
ATTENTION LE BOOKING ID IL FAUT RAJOUTER DES 0 POUR EVITER LE RAJOUT DE CHIFFRE POUR LES RESERVATIONS > 9 
*/

#define GREEN_LED_PIN 9
#define ORANGE_LED_PIN 0
#define RED_LED_PIN 1

#define LEFT_BUTTON_PIN 18
#define MIDDLE_BUTTON_PIN 5

#define RESERVATION_TIME 150 // in minutes

#define DELAY_BETWEEN_CHECKS_CONNECTION 30000 // in milliseconds
#define DELAY_BETWEEN_CHECKS_NO_RESERVATION 15000 // in milliseconds

#define RESERVATION_CANCELLED "2"
#define RESERVATION_ONGOING "1"


extern "C" int lwip_hook_ip6_input(void *p) {
  return 1; // Retourne 1 pour indiquer que le paquet IPv6 est accepté
}

ServerConnection server;
TaskHandle_t connectionTaskHandle = NULL;

ScreenOutput screenOutput;

Led greenLed = Led(GREEN_LED_PIN);
Led orangeLed = Led(ORANGE_LED_PIN);
Led redLed = Led(RED_LED_PIN);

Button leftButton = Button(LEFT_BUTTON_PIN);
Button middleButton = Button(MIDDLE_BUTTON_PIN);

const char* ntpServer = "pool.ntp.org";    // NTP server
const long gmtOffset_sec = 0;             // GMT offset in seconds
const int daylightOffset_sec = 3600;      // Daylight saving time offset in seconds

const char* deviceId = "00000002";

bool checkWifiConnection();

void mainSreen();

int checkReservations();

void changeReservationState(String reservationID, String newState);

void getReservationName(int reservationID, String& name, String& surname);

void setup() {

  Serial.println("Starting ResaLab...");

  screenOutput.begin();
  WiFi.begin(ssid, password);
  server.connect(serverUrl);
  uint8_t errorCode = server.checkConnection();

  if (errorCode == CODE_SUCCESS) {
    greenLed.blink(250);
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  log_v("Starting connection Handler");


  
}
uint64_t lastTimeConnectionCheck = 0;
uint64_t lastTimeReservationCheck = 0;
bool isConnected = false;
bool isToValid = true;
int currentReservationId = -1;

void loop() {
  
  if (millis() - lastTimeConnectionCheck > DELAY_BETWEEN_CHECKS_CONNECTION || !isConnected) {
    lastTimeConnectionCheck = millis();
    log_v("Checking connection...");
    isConnected = checkWifiConnection();
  }

  if (!isConnected) {
    return;
  }
  if (millis() - lastTimeReservationCheck > DELAY_BETWEEN_CHECKS_NO_RESERVATION) {
    lastTimeReservationCheck = millis();
    log_v("Checking reservations...");
    currentReservationId = checkReservations();
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

  log_d("Server response: %s", response.c_str());

  String serverResponseCode = response.substring(0, 3);

  if (serverResponseCode.equals(INVALID_RESERVATION)) {
    log_i("No reservation found.");
    greenLed.on();
    redLed.off();
    orangeLed.off();
    return -1;
  }

  int reservationId = response.substring(3, 8).toInt();
  bool reservationComing = response.substring(8, 9).toInt();
  int startReservationTimestamp = response.substring(9, 19).toInt();
  int endReservationTimestamp = response.substring(19, 29).toInt();

  log_d("Server response code: %s", serverResponseCode.c_str());
  log_d("reservationComing: %d", reservationComing);
  log_d("reservationId: %d", reservationId);
  log_d("startReservationTimestamp: %d", startReservationTimestamp);
  log_d("endReservationTimestamp: %d", endReservationTimestamp);

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
  } else if ( now > startReservationTimestamp && now < startReservationTimestamp + RESERVATION_TIME * 60) {
    greenLed.off();
    redLed.off();
    orangeLed.on();
    isToValid = true;
  } else {
    redLed.off();
    greenLed.on();
    orangeLed.off();
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
  server.reconnect();

  if (errorCode == ERROR_WIFI_DISCONNECTED) {
    log_v("Wifi disconnected");
    orangeLed.blink(1000);
  }

  if (errorCode == ERROR_SERVER_DISCONNECTED) {
    log_v("Server disconnected");
    orangeLed.blink(250, 250);
    orangeLed.blink(250, 1000);
  }

  log_d("Reconnection failed with error code: %d", errorCode);
  return false;
}

void getReservationName(int reservationID, String& name, String& surname) {
  int httpResponseCode = 0;

  char idBuffer[9
  ];
  snprintf(idBuffer, sizeof(idBuffer), "%08d", reservationID);

  // Prepare payload (here just the reservationID, adjust if your API expects more)
  String payload = String(idBuffer);

  // Send request to server (replace GET_RESERVATION_NAME with your actual command)
  int errorCode = server.sendRequest(GET_RESERVATION_NAME, strlen(GET_RESERVATION_NAME), payload.c_str(), payload.length(), &httpResponseCode);
  String response = server.getResponse();

  if (errorCode != CODE_SUCCESS) {
    log_e("Failed to get reservation name. Error code: %d", errorCode);
    name = "";
    surname = "";
    return;
  }

  log_d("Server response: %s", response.c_str());

  // Example: suppose response is "OKJohnDoe" (first 2 chars = code, then name, then surname)
  // Adjust parsing logic to your actual response format
  if (response.length() >= 4) {
    name = response.substring(2, response.length() / 2 + 1); // Example split
    surname = response.substring(response.length() / 2 + 1);
  } else {
    name = "";
    surname = "";
  }
}

void mainScreen() {

  
}
