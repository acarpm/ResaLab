#ifndef SCREENOUTPUT_H
#define SCREENOUTPUT_H

#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold10pt7b.h>

#include "logo.h"

#include <SPI.h>

#define GOOD_WIFI_RSSI -50
#define MEDIUM_WIFI_RSSI -60

#define WIFI_LOGO_X 280
#define WIFI_LOGO_Y 0
#define WIFI_LOGO_WIDTH 16
#define WIFI_LOGO_HEIGHT 16

#define MAX_CHARACTER 27

class ScreenOutput {

    private:
        const int SS = 10; // Chip Select pin for SPI
        const int DC = 8;  // Data/Command pin for SPI
        const int RES = 3;
        const int BUSY = 2;

        static const int SCK = 6;
        static const int MISO = 19;
        static const int MOSI = 7;

        int lastWifiState = -1;
        const uint8_t* wifiLogo[4] = {Wifi_Logo_0, Wifi_Logo_1, Wifi_Logo_2, Wifi_Logo_3};

        SPIClass SPIn;
        GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> 
        display;

        void initDisplay() {
            display.fillScreen(GxEPD_WHITE);
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&FreeSansBold10pt7b);
            display.display(true);
            display.fillScreenPartial(GxEPD_BLACK, 0, 0, 84, 128);
            display.fillScreenPartial(GxEPD_BLACK, 212, 0, 84, 128);
            display.fillScreenPartial(GxEPD_BLACK, 0, 124, 296, 4);
            display.drawInvertedBitmap(84, 0, CampusFab_Logo, 128, 127, GxEPD_BLACK);
            display.refresh(84, 0, 128, 127);
            display.display(true);
        } 

    public:
        ScreenOutput()
            : display(GxEPD2_290_BS(SS, DC, RES, BUSY))
        {}

        void begin() {
            log_d("Initializing display...");
            SPI.begin(SCK, MISO, MOSI, SS);
            display.init(115200, true, 50, false);
            display.setRotation(1);
            display.setFullWindow();
            initDisplay();
        }

        void resetScreen() {
            display.fillScreen(GxEPD_WHITE);
            display.display();
        }

        void showHour(int hour, int minute) {
            display.fillScreenPartial(GxEPD_BLACK, 0, 0, 58, 22);
            display.fillScreenPartial(GxEPD_WHITE, 0, 0, 56, 20);
            display.setCursor(2, 15);
            display.print(hour < 10 ? "0" : "");
            display.print(hour);
            display.print(":");
            display.print(minute < 10 ? "0" : "");
            display.print(minute);
            display.display(true);
        }

        void showInternetState(bool connected, int rssi = 0) {
            int internetState = 0;
            if (connected) {
                if (rssi >= GOOD_WIFI_RSSI) {
                    internetState = 3;
                } else if (rssi >= MEDIUM_WIFI_RSSI) {
                    internetState = 2;
                } else {
                    internetState = 1;
                }
            }
            
            if (internetState == lastWifiState) {
                return;
            }
            lastWifiState = internetState;
            display.fillScreenPartial(GxEPD_BLACK, WIFI_LOGO_X - 2, WIFI_LOGO_Y - 2, WIFI_LOGO_WIDTH + 4, WIFI_LOGO_HEIGHT + 6);
            display.fillScreenPartial(GxEPD_WHITE, WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_WIDTH, WIFI_LOGO_HEIGHT + 2);
            display.drawInvertedBitmap(WIFI_LOGO_X, WIFI_LOGO_Y, wifiLogo[internetState], WIFI_LOGO_WIDTH, WIFI_LOGO_HEIGHT, GxEPD_BLACK);
            display.refresh(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_WIDTH, WIFI_LOGO_HEIGHT);
            display.display(true);
        }

        void showText(String text, String text2 = "") {
            display.fillScreenPartial(GxEPD_WHITE, 0, 22, 296, 106);
            if (text.length() > MAX_CHARACTER) {
                log_e("To much characters");
                return;
            }
            if (text2.length() > MAX_CHARACTER) {
                log_e("To much characters");
                return;
            }

            display.setCursor(2, 66);
            display.print(text);
            display.setCursor(2, 85);
            display.print(text2);
            display.display(true);
        }

        void showProgressionBar(int percent) {
            display.fillScreenPartial(GxEPD_BLACK, 2, 106, 292, 20);
            display.fillScreenPartial(GxEPD_WHITE, 3, 107, 290, 18);

            int width = (percent * 288) / 100;
            display.fillScreenPartial(GxEPD_BLACK, 4, 108, width, 16);
        }

};


#endif // SCREENOUTPUT_H