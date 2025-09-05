#ifndef SCREENOUTPUT_H
#define SCREENOUTPUT_H

#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#include <SPI.h>

class ScreenOutput {

    private:
        const int SS = 10; // Chip Select pin for SPI
        const int DC = 8;  // Data/Command pin for SPI
        const int RES = 3;
        const int BUSY = 2;

        static const int SCK = 6;
        static const int MISO = -1;
        static const int MOSI = 7;

        SPIClass SPIn;
        GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display;

        void initDisplay() {
            display.fillScreen(GxEPD_WHITE);
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&FreeMonoBold9pt7b);
            display.setCursor(0, 20);
            display.print("ResaLab");
            display.display();
            log_d("Resalab");
        } 

    public:
        ScreenOutput()
            : display(GxEPD2_290_C90c(SS, DC, RES, BUSY))
        {}

        void begin() {
            log_d("Initializing display...");
            SPI.begin(SCK, MISO, MOSI, SS);
            display.init(115200, true, 50, false);
            display.setRotation(1);
            display.setFullWindow();
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