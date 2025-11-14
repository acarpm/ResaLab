#ifndef SCREENOUTPUT_H
#define SCREENOUTPUT_H

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#include "logo.h"

#include <SPI.h>

class ScreenOutput {

    private:
        const int SS = 10; // Chip Select pin for SPI
        const int DC = 8;  // Data/Command pin for SPI
        const int RES = 3;
        const int BUSY = 2;

        static const int SCK = 6;
        static const int MISO = 19;
        static const int MOSI = 7;

        SPIClass SPIn;
        GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> 
        display;

        void initDisplay() {
            display.fillScreen(GxEPD_WHITE);
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&FreeMonoBold9pt7b);
            display.setCursor(20, 20);
            display.print("ResaLab");
            display.display(true);
            display.writeImage(Wifi_Logo_3, 50, 50, 16, 16);
            display.refresh(50, 50, 16, 16);
            delay(1000);
            display.writeImage(Wifi_Logo_3, 50, 50, 16, 16, true);
            display.refresh(50, 50, 8, 8);
            for (int i = 0; i < 10; i++) {
                display.setCursor(20, 50);
                display.print(i);
                display.writeScreenBuffer();
                display.display(true);
            }
            log_d("Resalab");
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

        void showHour(int hour, int minute) {
            // TODO
        }

        void showInternetState(bool connected) {
            // TODO
        }

        void showLoading() {
            // TODO    
        }

};


#endif // SCREENOUTPUT_H