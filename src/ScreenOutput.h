#ifndef SCREENOUTPUT_H
#define SCREENOUTPUT_H

#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold10pt7b.h>

#include "logo.h"

#include <SPI.h>

#define SS_PIN 10   // Chip Select pin for SPI
#define DC_PIN 8    // Data/Command pin for SPI
#define RES_PIN 3   // Reset pin for SPI
#define BUSY_PIN 2  // Busy pin for SPI

#define SCK_PIN 6   // Serial Clock pin for SPI
#define MISO_PIN 19 // Master In Slave Out pin for SPI. We are not supposed to use it, but must define it.
#define MOSI_PIN 7  // Master Out Slave In pin for SPI

// WiFi RSSI thresholds (in dBm)
#define GOOD_WIFI_RSSI -50
#define MEDIUM_WIFI_RSSI -60

// WiFi logo dimensions and position
#define WIFI_LOGO_X 280
#define WIFI_LOGO_Y 0
#define WIFI_LOGO_WIDTH 16
#define WIFI_LOGO_HEIGHT 16

// CampusFab logo dimensions and position
#define CAMPUSFAB_LOGO_WIDTH 128
#define CAMPUSFAB_LOGO_HEIGHT 127
#define CAMPUSFAB_LOGO_X 84
#define CAMPUSFAB_LOGO_Y 0

// Hour display box
#define HOUR_X 0
#define HOUR_Y 0
#define HOUR_WIDTH 58
#define HOUR_HEIGHT 22

// Maximum characters per line for text display
#define MAX_CHARACTER 27

// Text box dimensions and position
#define TEXT_BOX_X 0
#define TEXT_BOX_Y 22
#define TEXT_BOX_WIDTH 296
#define TEXT_BOX_HEIGHT 106
#define TEXT_LINE_HEIGHT 19
#define TEXT_START_X (TEXT_BOX_X + 2)
#define TEXT_START_Y (TEXT_BOX_Y + 44)

// Progress bar dimensions and position
#define PROGRESS_BAR_X 2
#define PROGRESS_BAR_Y 106
#define PROGRESS_BAR_WIDTH 292 // MUST BE > 4
#define PROGRESS_BAR_HEIGHT 20 // MUST BE > 4


class ScreenOutput {

    private:

        int lastWifiState = -1;
        // Wifi logos for different signal strengths available in logo.h.
        const uint8_t* wifiLogo[4] = {Wifi_Logo_0, Wifi_Logo_1, Wifi_Logo_2, Wifi_Logo_3};


        GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display;

        /**
         * @brief Initialize and render the e-paper display with the CampusFab logo.
         *
         * This function performs the necessary initialization and first-frame rendering
         * for the e-paper display used by the application.
         * Clears the display to white and sets default text drawing properties (text color and font).
         * Performs an initial full display update.
         * Draws the CampusFab logo.
         * 
         * @note This function relies on several external objects.
         *      FreeSansBold10pt7b  : font used for text rendering.
         *      CampusFab_Logo      : bitmap data for the logo.
         * 
         */
        void initDisplay() {
            display.fillScreen(GxEPD_WHITE);

            // Other font available in Adafruit_GFX library 
            display.setFont(&FreeSansBold10pt7b);
            display.setTextColor(GxEPD_BLACK);
            
            display.display();

            // Display CampusFab logo Filling black first then showing logo
            display.fillScreenPartial(GxEPD_BLACK, 0, 0, GxEPD2_290_BS::HEIGHT, GxEPD2_290_BS::WIDTH);
            display.fillScreenPartial(GxEPD_WHITE, CAMPUSFAB_LOGO_X, CAMPUSFAB_LOGO_Y, CAMPUSFAB_LOGO_WIDTH, CAMPUSFAB_LOGO_HEIGHT);
            // CampusFab logo in logo.h
            display.drawInvertedBitmap(CAMPUSFAB_LOGO_X, CAMPUSFAB_LOGO_Y, CampusFab_Logo, CAMPUSFAB_LOGO_WIDTH, CAMPUSFAB_LOGO_HEIGHT, GxEPD_BLACK);
            display.refresh(true);
            display.display(true);
        } 

    public:
        ScreenOutput()
            : display(GxEPD2_290_BS(SS_PIN, DC_PIN, RES_PIN, BUSY_PIN))
        {}

        /**
         * @brief Initialize the SPI bus and the display.
         *
         * Intended usage: Call once during system startup (for example in setup()) before performing 
         * any drawing or display updates.
         * 
         */
        void begin() {
            log_d("Initializing display...");
            SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
            display.init(115200, true, 50, false);
            display.setRotation(1);
            display.setFullWindow();
            initDisplay();
        }

        /**
         * @brief Clear the e-paper display and refresh the physical screen.
         * 
         */
        void resetScreen() {
            display.fillScreen(GxEPD_WHITE);
            display.display();
        }

        /**
         * @brief Displays the current hour and minute on the e-paper display.
         * 
         * This function renders a time display in HH:MM format with leading zeros.
         * 
         * @param hour The hour value to display
         * @param minute The minute value to display
         * 
         */
        void showHour(int hour, int minute) {
            // BOX
            display.fillScreenPartial(GxEPD_BLACK, HOUR_X, HOUR_Y, HOUR_WIDTH, HOUR_HEIGHT);
            display.fillScreenPartial(GxEPD_WHITE, HOUR_X, HOUR_Y, HOUR_WIDTH - 2, HOUR_HEIGHT - 2);

            if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
                log_e("Invalid time: %02d:%02d", hour, minute);
                display.display(true);
                return;
            }

            display.setCursor(HOUR_X + 2, HOUR_Y + 15);
            display.print(hour < 10 ? "0" : "");
            display.print(hour);
            display.print(":");
            display.print(minute < 10 ? "0" : "");
            display.print(minute);
            display.display(true);
        }

        /**
         * @brief Displays the current internet connection state on the screen with a WiFi icon.
         * 
         * @param connected Boolean indicating whether the device is connected to the internet.
         * @param rssi The Received Signal Strength Indicator value in dBm.
         * 
         */
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
            
            // No change in state, no need to update display, saving time and power.
            if (internetState == lastWifiState) {
                return;
            }

            lastWifiState = internetState;

            // BOX
            display.fillScreenPartial(GxEPD_BLACK, WIFI_LOGO_X - 2, WIFI_LOGO_Y - 2, WIFI_LOGO_WIDTH + 4, WIFI_LOGO_HEIGHT + 6);
            display.fillScreenPartial(GxEPD_WHITE, WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_WIDTH, WIFI_LOGO_HEIGHT + 2);

            display.drawInvertedBitmap(WIFI_LOGO_X, WIFI_LOGO_Y, wifiLogo[internetState], WIFI_LOGO_WIDTH, WIFI_LOGO_HEIGHT, GxEPD_BLACK);
            
            display.refresh(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_WIDTH, WIFI_LOGO_HEIGHT);
            display.display(true);
        }

        /**
         * @brief Displays text on the e-paper display with optional second line.
         * 
         * 
         * @param text The primary text string to display on the first line.
         * @param text2 Optional secondary text string to display on the second line.
         * .
         */
        void showText(String text, String text2 = "") {
            display.fillScreenPartial(GxEPD_WHITE, TEXT_BOX_X, TEXT_BOX_Y, TEXT_BOX_WIDTH, TEXT_BOX_HEIGHT);

            if (text.length() > MAX_CHARACTER) {
                log_e("Too many characters for first line");
                return;
            }
            if (text2.length() > MAX_CHARACTER) {
                log_e("Too many characters for second line");
                return;
            }

            display.setCursor(TEXT_START_X, TEXT_START_Y);
            display.print(text);
            display.setCursor(TEXT_START_X, TEXT_START_Y + TEXT_LINE_HEIGHT);
            display.print(text2);
            display.display(true);
        }

        /**
         * @brief Displays a progress bar on the screen at a fixed position.
         * 
         * @param percent The progress percentage to display, automatically clamped to the range [0, 100].
         * 
         */
        void showProgressionBar(int percent) {
            // Clamp percent to [0, 100]
            if (percent < 0) percent = 0;
            if (percent > 100) percent = 100;

            // BOX
            display.fillScreenPartial(GxEPD_BLACK, PROGRESS_BAR_X, PROGRESS_BAR_Y, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT);
            display.fillScreenPartial(GxEPD_WHITE, PROGRESS_BAR_X + 1, PROGRESS_BAR_Y + 1, PROGRESS_BAR_WIDTH - 2, PROGRESS_BAR_HEIGHT - 2);

            int width = (percent * PROGRESS_BAR_WIDTH - 4) / 100;
            display.fillScreenPartial(GxEPD_BLACK, PROGRESS_BAR_X + 4, PROGRESS_BAR_Y + 2, width, PROGRESS_BAR_HEIGHT - 4);
        }

};


#endif // SCREENOUTPUT_H