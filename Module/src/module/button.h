#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <Arduino.h>

class Button {
    private : 
        int pin;

    public : 
        Button(int pin) : pin(pin) {
            pinMode(pin, INPUT);
        }

        int isPressed() {
            int reading = digitalRead(pin);

            return reading;
        }
};

#endif