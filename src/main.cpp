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

#define RESERVATION_TIME 15 // in minutes

#define DELAY_BETWEEN_CHECKS_CONNECTION 30000 // in milliseconds
#define DELAY_BETWEEN_CHECKS_NO_RESERVATION 15000 // in milliseconds
#define DELAY_BETWEEN_CHECKS_HOURS 15000 // in milliseconds

#define TIME_BEFORE_SHOWING_RESERVATION 180

#define RESERVATION_CANCELLED "4"
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
const long gmtOffset_sec = 3600;             // GMT offset in seconds
const int daylightOffset_sec = 3600;      // Daylight saving time offset in seconds

const char* deviceId = "00000002";

bool checkWifiConnection();

void mainSreen();

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
  uint8_t errorCode = server.checkConnection();


  while (!isConnected) {
    isConnected = checkWifiConnection();
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  screenOutput.resetScreen();

  getTime(&currentHour, &currentMinute);

  screenOutput.showHour(currentHour, currentMinute);
  screenOutput.showInternetState(true, WiFi.RSSI());
}
uint64_t lastTimeConnectionCheck = 0;
uint64_t lastTimeReservationCheck = 0;
uint64_t lastTimeScreenUpdated = 0;

bool isToValid = true;
int currentReservationId = -1;



void loop() {
  int currentHour, currentMinute;

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
    getTime(&currentHour, &currentMinute);

    screenOutput.showHour(currentHour, currentMinute);
    screenOutput.showInternetState(true, WiFi.RSSI());
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
    screenOutput.showText("Aucune reservations");
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
    return -2;
  }

  log_v("Current timestamp: %ld", now);

  if (!reservationComing && now > startReservationTimestamp && now < endReservationTimestamp) {
    redLed.on();
    greenLed.off();
    orangeLed.off();

    String name = "";
    String surname = "";
    getReservationName((String)currentReservationId, name, surname);

    screenOutput.showText("Reservation en cours : ", name + " " + surname);
    
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
      int timeBeforeReservation = startReservationTimestamp - now;

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

void getReservationName(String reservationID, String& name, String& surname) {
  int httpResponseCode = 0;

  String payload = "";
  for (int i = reservationID.length(); i < 8; i++) {
    payload += "0";
  }
  payload += reservationID;

  int errorCode = server.sendRequest(GET_RESERVATION_NAME, strlen(GET_RESERVATION_NAME), payload.c_str(), payload.length(), &httpResponseCode);
  String response = server.getResponse();

  if (errorCode != CODE_SUCCESS) {
    log_e("Failed to get reservation name. Error code: %d", errorCode);
    name = "";
    surname = "";
    return;
  }

  log_d("Server response: %s", response.c_str());

  response.trim();

  int sep = response.indexOf('&');
  if (sep < 0) {
    name = response;
    surname = "";
    return;
  }

  name = response.substring(0, sep);
  surname = response.substring(sep + 1);
}

void getTime(int* hours, int* minutes) {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  *hours = timeinfo->tm_hour;
  *minutes = timeinfo->tm_min;
}

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
