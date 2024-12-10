#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>


// Button debounce code
const uint8_t DEBOUNCE_DELAY = 20; // in milliseconds

struct Button {
    // state variables
    uint8_t  pin;
    bool     lastReading;
    uint32_t lastDebounceTime;
    uint16_t state;

    // methods determining the logical state of the button
    bool pressed()                { return state == 1; }
    bool released()               { return state == 0xffff; }
    bool held(uint16_t count = 0) { return state > 1 + count && state < 0xffff; }

    // method for reading the physical state of the button
    void read() {
        // reads the voltage on the pin connected to the button
        bool reading = digitalRead(pin);

        // if the logic level has changed since the last reading,
        // we reset the timer which counts down the necessary time
        // beyond which we can consider that the bouncing effect
        // has passed.
        if (reading != lastReading) {
            lastDebounceTime = millis();
        }

        // from the moment we're out of the bouncing phase
        // the actual status of the button can be determined
        if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
            // The read pin is pulled-down
            bool pressed = reading == HIGH;
            if (pressed) {
                     if (state  < 0xfffe) state++;
                else if (state == 0xfffe) state = 2;
            } else if (state) {
                state = state == 0xffff ? 0 : 0xffff;
            }
        }

        // finally, each new reading is saved
        lastReading = reading;
    }
};


// Module definitions for communication with the main module.
#define STRIKE        18    // Pin to indicate a strike in-game
#define SOLVED        19    // Pin to indicate the module has been solved

// Declaration for SSD1306 display connected using I2C
#define OLED_SCL        22
#define OLED_SDA        21
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS_1 0x3C
#define SCREEN_ADDRESS_2 0x3D
Adafruit_SSD1306 display_1(128, 64, &Wire, OLED_RESET);
Adafruit_SSD1306 display_2(128, 64, &Wire, OLED_RESET);


// ***********  BITMAPS   ***********
// Strike pixel art, 128x64px
const unsigned char strike_screen [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 
	0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf8, 0x00, 0x00, 
	0x00, 0x00, 0x1f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf0, 0x00, 0x00, 
	0x00, 0x00, 0x0f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xc0, 0x00, 0x7f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7f, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf8, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x0f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x0f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Completed pixel art, 128x64px
const unsigned char solved_screen [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xf0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfc, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf8, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xe0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x00, 0x03, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xff, 0xe0, 0x00, 0x1f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xff, 0xe0, 0x00, 0x1f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xff, 0xf0, 0x00, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xff, 0xfc, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0xff, 0xfe, 0x01, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x03, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x87, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xc7, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Small battery pixel art, 15x30px
const unsigned char small_battery [] PROGMEM = {
	0x1f, 0xf0, 0x1f, 0xf0, 0x1f, 0xf0, 0x1f, 0xf0, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 
	0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 
	0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 
	0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe
};

// Symbol for No.01 in pseudo-timer, 24x24px
const unsigned char timer_symbol_00 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x7e, 0x00, 0x00, 0xe7, 0x00, 0x01, 
	0xc3, 0x80, 0x03, 0x81, 0xc0, 0x07, 0x00, 0xe0, 0x06, 0x00, 0x60, 0x06, 0x00, 0x60, 0x06, 0x00, 
	0x60, 0x06, 0x00, 0x60, 0x06, 0x00, 0x60, 0x07, 0x00, 0xe0, 0x03, 0x81, 0xc0, 0x01, 0xc3, 0x80, 
	0x00, 0xff, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 
	0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Symbol for No.02 in pseudo-timer, 24x24px
const unsigned char timer_symbol_01 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x66, 0x00, 0x00, 
	0x66, 0x00, 0x00, 0x42, 0x00, 0x00, 0xc3, 0x00, 0x00, 0xc3, 0x00, 0x01, 0x81, 0x80, 0x01, 0x81, 
	0x80, 0x03, 0xff, 0xc0, 0x03, 0xff, 0xc0, 0x06, 0x18, 0x60, 0x06, 0x18, 0x60, 0x0c, 0x18, 0x30, 
	0x0c, 0x18, 0x30, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x30, 0x18, 0x0c, 0x30, 
	0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Symbol for No.03 in pseudo-timer, 24x24px
const unsigned char timer_symbol_02 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf0, 0x03, 0xfe, 0x40, 0x07, 0xfe, 0x40, 0x07, 
	0xfe, 0x40, 0x0f, 0xfe, 0x40, 0x0f, 0xfe, 0x40, 0x0f, 0xfe, 0x40, 0x0f, 0xfe, 0x40, 0x07, 0xfe, 
	0x40, 0x07, 0xfe, 0x40, 0x03, 0xfe, 0x40, 0x00, 0xfe, 0x40, 0x00, 0x3e, 0x40, 0x00, 0x00, 0x40, 
	0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Symbol for No.04 in pseudo-timer, 24x24px
const unsigned char timer_symbol_03 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x07, 0xff, 0x80, 0x0e, 0x01, 0xe0, 0x18, 
	0x00, 0x70, 0x18, 0x00, 0x30, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x30, 
	0x18, 0x00, 0x30, 0x18, 0x00, 0x30, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 
	0x00, 0x00, 0x18, 0x18, 0x00, 0x30, 0x18, 0x00, 0x70, 0x0e, 0x01, 0xe0, 0x07, 0xff, 0x80, 0x01, 
	0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Symbol for No.05 in pseudo-timer, 24x24px
const unsigned char timer_symbol_04 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xff, 0xfc, 0x33, 0xff, 0xfc, 0x31, 0x80, 0x18, 0x30, 
	0xc0, 0x30, 0x30, 0xc0, 0x30, 0x30, 0x60, 0x60, 0x30, 0x60, 0x60, 0x30, 0x30, 0xc0, 0x3f, 0xf0, 
	0xc0, 0x3f, 0xf9, 0x80, 0x30, 0x19, 0x80, 0x30, 0x0f, 0x00, 0x30, 0x3f, 0xc0, 0x30, 0x76, 0xe0, 
	0x30, 0xe6, 0x70, 0x30, 0xc6, 0x30, 0x31, 0x86, 0x18, 0x31, 0x86, 0x18, 0x33, 0x06, 0x0c, 0x33, 
	0x06, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Symbol for No.06 in pseudo-timer, 24x24px
const unsigned char timer_symbol_05 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x70, 0x3c, 0x00, 0xd8, 0x26, 0x01, 0x98, 0x02, 
	0x03, 0x30, 0x03, 0x02, 0x60, 0x03, 0x06, 0xc0, 0x03, 0x07, 0x80, 0x01, 0x8e, 0x00, 0x01, 0xbc, 
	0x00, 0x01, 0xfc, 0x0c, 0x01, 0xfc, 0x1c, 0x03, 0x8c, 0x3c, 0x0f, 0x0f, 0xec, 0x19, 0x07, 0xcc, 
	0x13, 0x00, 0x0c, 0x36, 0x00, 0x18, 0x24, 0x00, 0x30, 0x3c, 0x00, 0xe0, 0x18, 0x07, 0xc0, 0x00, 
	0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Symbol for No.07 in pseudo-timer, 24x24px
const unsigned char timer_symbol_06 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 
	0x7e, 0x00, 0x00, 0x7e, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x1f, 0xff, 0xf8, 0x3f, 0xff, 
	0xfc, 0x0f, 0xff, 0xf0, 0x03, 0xff, 0xc0, 0x01, 0xff, 0x80, 0x03, 0xff, 0xc0, 0x03, 0xff, 0xc0, 
	0x07, 0xff, 0xe0, 0x07, 0xe7, 0xe0, 0x0f, 0xc3, 0xf0, 0x0f, 0x81, 0xf0, 0x0f, 0x00, 0xf0, 0x0c, 
	0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Symbol for No.08 in pseudo-timer, 24x24px
const unsigned char timer_symbol_07 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x00, 0x00, 0xc3, 0x00, 0x00, 0x00, 0x00, 0x03, 
	0xff, 0x80, 0x0f, 0xff, 0xc0, 0x1c, 0x00, 0xe0, 0x18, 0x00, 0x70, 0x18, 0x00, 0x30, 0x18, 0x00, 
	0x38, 0x00, 0x00, 0x18, 0x00, 0x3f, 0xf8, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x18, 0x18, 0x00, 0x38, 
	0x18, 0x00, 0x30, 0x18, 0x00, 0x70, 0x1c, 0x00, 0xe0, 0x0f, 0xff, 0xc0, 0x03, 0xff, 0x80, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Symbol for No.09 in pseudo-timer, 24x24px
const unsigned char timer_symbol_08 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x01, 0xc0, 0x00, 0x03, 0x87, 0x00, 0x07, 
	0x1f, 0xc0, 0x0e, 0x18, 0xc0, 0x0c, 0x30, 0x60, 0x18, 0x30, 0x60, 0x18, 0x60, 0x30, 0x30, 0x60, 
	0x30, 0x30, 0x60, 0x30, 0x30, 0x60, 0x30, 0x30, 0x60, 0x30, 0x30, 0x60, 0x30, 0x18, 0x30, 0x60, 
	0x18, 0x38, 0xe4, 0x0c, 0x1c, 0xcc, 0x0e, 0x0f, 0xdc, 0x07, 0x07, 0xf8, 0x01, 0xfe, 0x70, 0x00, 
	0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Symbol for No.10 in pseudo-timer, 24x24px
const unsigned char timer_symbol_09 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x01, 0xff, 0x80, 0x03, 0x81, 0xc0, 0x06, 
	0x00, 0x60, 0x0c, 0x00, 0x30, 0x18, 0x00, 0x18, 0x38, 0x00, 0x1c, 0x30, 0x00, 0x0c, 0x30, 0x00, 
	0x0c, 0x30, 0x00, 0x0c, 0x30, 0x00, 0x0c, 0x38, 0x00, 0x1c, 0x1c, 0x00, 0x38, 0x0e, 0x00, 0x70, 
	0x07, 0x00, 0xe0, 0x03, 0xc3, 0xc0, 0x00, 0xc3, 0x00, 0x20, 0xc3, 0x04, 0x3f, 0xc3, 0xfc, 0x3f, 
	0xc3, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Array of all pseudo-timer bitmaps for convenience. (Total bytes used to store images in PROGMEM = 960)
const int timer_bitmaps_LEN = 10;
const unsigned char* timer_bitmaps[10] = {
	timer_symbol_00,
  timer_symbol_01,
  timer_symbol_02,
  timer_symbol_03,
  timer_symbol_04,
  timer_symbol_05,
  timer_symbol_06,
  timer_symbol_07,
  timer_symbol_08,
  timer_symbol_09,
};


// Definitions for LED strip
#define LED_STRIP_PIN       17
#define NUM_BUTTON_LEDS     4
#define NUM_STRIP_LEDS      5
#define NUM_LEDS            (NUM_BUTTON_LEDS + NUM_STRIP_LEDS)
#define M_BRIGHT            50    // Maximum value for the LEDs to turn on (up to 255).
Adafruit_NeoPixel module_leds = Adafruit_NeoPixel(NUM_LEDS, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

// Game definitions and variables
#define MAIN_BUTTON       15

Button the_button      = { MAIN_BUTTON, LOW, 0, 0 };
bool module_solution = false;     // Variable to indicate if the module is solved
byte minutes_ones;
byte seconds_tens;
byte seconds_ones;
unsigned long current_time;
unsigned long last_action_time;
bool button_pressed;

char button_text[4][9] = {"Detonate","Abort", "Hold", "Press"};
const uint32_t light_colors[6] = {
        module_leds.Color(M_BRIGHT, M_BRIGHT, M_BRIGHT),  // 0 - White   - Light Strip & Button
        module_leds.Color(M_BRIGHT, 0, 0),                // 1 - Red     - Light Strip & Button
        module_leds.Color(0, 0, M_BRIGHT),                // 2 - Blue    - Light Strip & Button
        module_leds.Color(M_BRIGHT, M_BRIGHT, 0),         // 3 - Yellow  - Light Strip & Button
        module_leds.Color(M_BRIGHT, 0, M_BRIGHT),         // 4 - Magenta - Light Strip
        module_leds.Color(0, M_BRIGHT, 0),                // 5 - Green   - Light Strip
    };
byte b_color;              // Button color (Random number from 0 to 3 (inclusive)) from light_colors
byte button_game_text;               // Button text (Random number from 0 to 3 (inclusive)) from button_texts
byte light_strip_color;    // Light strip color (Random number from 0 to 5 (inclusive)) from light_colors
byte num_batteries;        // Number of batteries (Up to 3 batteries, the number should be in base 1)
byte shape;                // Shape index (Random number from 0 to 5 (inclusive))
// Shapes are indexed as follows:
//    0 - Empty triangle
//    1 - Filled triangle
//    2 - Empty circle
//    3 - Filled circle
//    4 - Empty square
//    5 - Filled square
bool game_solution;       // Indicate the button mode used to solve the module.
// True for hold and release immediately, and false for hold and wait for instructions.


// Display Functions
// S1 or S2 indicates for which screen the function works.

void draw_centered_text_S1(char display_text[], byte text_size, byte y_offset, bool vertical_center = false) {
  // Needs a clear display before the function, and a display after the function
  display_1.setTextSize(text_size);
  display_1.setTextColor(SSD1306_WHITE);
  int16_t  x1, y1;
  uint16_t w, h;

  display_1.getTextBounds(display_text, 0, 0, &x1, &y1, &w, &h);
  display_1.setCursor((128/2)-(w/2), y_offset);
  if (vertical_center) {
    display_1.setCursor((128/2)-(w/2), (64/2)-(h/2));
  }

  display_1.println(display_text);
}


void draw_rectangle_S1(byte width, byte height, byte thickness, byte x_0 = 0, byte y_0 = 0) {
  for (int i = 0; i < thickness; i++) {
    display_1.drawRect(x_0 + i, y_0 + i, width - (2*i), height - (2*i), SSD1306_WHITE);
  }
}


void draw_rectangle_S2(byte width, byte height, byte thickness, byte x_0 = 0, byte y_0 = 0) {
  for (int i = 0; i < thickness; i++) {
    display_2.drawRect(x_0 + i, y_0 + i, width - (2*i), height - (2*i), SSD1306_WHITE);
  }
}


void draw_circle_S2(byte radius, byte thickness, byte x_0 = 0, byte y_0 = 0) {
  display_2.fillCircle(x_0, y_0, radius, SSD1306_WHITE);
  display_2.fillCircle(x_0, y_0, radius - (thickness), SSD1306_BLACK);
}


void draw_triangle_S2(int x_0, int y_0, int x_1, int y_1, int x_2, int y_2, int thickness, int x = 0, int y = 0) {
  display_2.fillTriangle(x_0 + x, y_0 + y, x_1 + x, y_1 + y, x_2 + x, y_2 + y, SSD1306_WHITE);
  display_2.fillTriangle(
    (x_0 + x + 2*thickness), (y_0 + y - thickness),
    (x_1 + x), (y_1 + y + 2*thickness),
    (x_2 + x - 2*thickness), (y_2 + y - thickness),
    SSD1306_BLACK);
}


void display_phase_1(char word[], byte shape_index, byte n_batteries) {
  // Display configuration for the first stage of the module
  // (Before Pressing the button)
  
  // Clear both displays
  display_2.clearDisplay();
  display_1.clearDisplay();

  // Button word
  if (word != button_text[0]) {
    // Draw the word with size 3 for words that are not Detonate.
    draw_centered_text_S1(word, 3, 0, true);   // Draw the word in the display
  }
  else {
    // If the word is Detonate, write it in size 2 (else is too large to fit in one line)
    draw_centered_text_S1(word, 2, 0, true);   // Draw the word in the display
  }

  // Shape information
  draw_rectangle_S2(65, 64, 2, 0, 0);    // Rectangle enclosing the shape
  switch (shape_index) {
  // Choose which shape to display based on the game setup
    case 0:
      draw_triangle_S2(6 - 15, 28, 6, 1, 6 + 15, 28, 4, 27, 17);        // Draw an empty triangle
      break;
    case 1:
      display_2.fillTriangle(6 + (27 - 15), 45, 6 + 27, 18, 6 + (27 + 15), 45, SSD1306_WHITE);    // Filled triangle
      break;
    case 2:
      draw_circle_S2(14, 4, 6 + 27, 17 + 15);                           // Draw an empty circle
      break;
    case 3:
      display_2.fillCircle(6 + 27, 17 + 15, 14, SSD1306_WHITE);         // Draw a filled circle
      break;
    case 4:
      draw_rectangle_S2(30, 30, 4, 6 + (54/2 - 15), 17);                // Draw an empty square
      break;
    case 5:
      display_2.fillRect(6 + (54/2 - 15), 17, 30, 30, SSD1306_WHITE);   // Draw a filled square
      break;
    default:
      Serial.println("The shape index is not between boundaries");
      break;
  }

  // Battery information
  draw_rectangle_S2(65, 64 , 2, 63, 0);   // Rectangle enclosing the batteries
  if (n_batteries != 2) {
    display_2.drawBitmap(69 + (4 + 15), 17, small_battery, 15, 30, SSD1306_WHITE);    // Display 1 battery
  }
  if (n_batteries == 2) {
    display_2.drawBitmap(65 + 13, 17, small_battery, 15, 30, SSD1306_WHITE);    // Display 2 batteries
    display_2.drawBitmap(69 + 13 + (4 + 15), 17, small_battery, 15, 30, SSD1306_WHITE);
  }
  if (n_batteries > 2) {
    display_2.drawBitmap(69 + 2*(4 + 15), 17, small_battery, 15, 30, SSD1306_WHITE);  // Display 3 batteries
    display_2.drawBitmap(69, 17, small_battery, 15, 30, SSD1306_WHITE);
  }

  // Display the new information for both displays
  display_1.display();
  display_2.display();
}


void display_hold_and_wait() {
  // Display configuration for the first stage of the module
  // (Before Pressing the button)

  display_1.clearDisplay();

  // Draw pseudo-timer in screen
  // Draw the minutes ones
  draw_rectangle_S1(24 + 8, 24 + 8, 2, 14 - 4, 20 - 4);
  display_1.drawBitmap(14, 20, timer_bitmaps[minutes_ones], 24, 24, SH110X_WHITE);

  // Draw the seconds tens
  draw_rectangle_S1(24 + 8, 24 + 8, 2, (2*14 + 24) - 4, 20 - 4);
  display_1.drawBitmap(2*14 + 24, 20, timer_bitmaps[seconds_tens], 24, 24, SH110X_WHITE);

  // Draw the seconds ones
  draw_rectangle_S1(24 + 8, 24 + 8, 2, (3*14 + 2*24) - 4, 20 - 4);
  display_1.drawBitmap(3*14 + 2*24, 20, timer_bitmaps[seconds_ones], 24, 24, SH110X_WHITE);

  // Display timer
  display_1.display();
}


void display_strike() {
  // 
  display_1.clearDisplay();

  // Strike display
  display_1.drawBitmap(0, 0, strike_screen, 128, 64, SSD1306_WHITE);   // Display the first battery

  display_1.display();
}


void display_solved_module() {
  // 
  display_1.clearDisplay();

  // Solved display
  display_1.drawBitmap(0, 0, solved_screen, 128, 64, SSD1306_WHITE);   // Display the first battery
  
  display_1.display();
}


// LED strip functions

void turn_off_lights() {
  for (int i = 0; i > NUM_LEDS; i++){
    module_leds.setPixelColor(i, 0, 0, 0);
  }
  module_leds.show();
  delay(40);
}


void game_leds(byte button_color, bool light_strip = false, byte light_strip_color = 0) {
  // Turns on the LEDs for the button and the light strip

  // Set button LEDs
  for (int i = 0; i < NUM_BUTTON_LEDS; i++) {
    module_leds.setPixelColor(i, light_colors[button_color]);
  }

  // Set light strip LEDs
  for (int i = 0; i < NUM_STRIP_LEDS; i++) {
    if (light_strip == true) {
      module_leds.setPixelColor(i + NUM_STRIP_LEDS - 1, light_colors[light_strip_color]);
    }
    else {
      module_leds.setPixelColor(i + NUM_STRIP_LEDS - 1, 0, 0, 0);
    }
  }

  // Show the color of the button and the light strip LEDs
  module_leds.show();
  delay(40);
}


// Game functions

bool release_immediately() {
  // Conditional statements to determine if the button has to be pressed and released immediately

  if (b_color == 2 && button_game_text == 1) {
    // If the button is blue and the button says "Abort", hold and wait.
    return false;
  }
  else if (num_batteries > 1 && button_game_text == 0) {
    // If there is more than 1 battery on the bomb and the button says "Detonate", hold and release.
    return true;
  }
  else if (b_color == 0 && shape == 3) {
    // If the button is white and the shape is a "filled" circle, hold and wait.
    return false;
  }
  else if (num_batteries > 2 && shape == 0) {
    // If there are more than 2 batteries on the bomb and the shape is an "empty" triangle, hold and release.
    return true;
  }
  else if (b_color == 3) {
    // If the button is yellow, hold and wait.
    return false;
  }
  else if (b_color == 1 && button_game_text == 2) {
    // If the button is red and the button says "Hold", hold and release.
    return true;
  }
  else {
    return false;
  }
}


void game_setup() {
  // Generate the game states
  b_color = random(4);
  button_game_text =  random(4);
  light_strip_color = random(6);
  num_batteries = random(1, 4);   // Chose up to 3 batteries
  shape = random(6);

  game_solution = release_immediately();
}


void hold_and_release() {
  
  last_action_time = millis();
  current_time = millis();

  while (current_time < last_action_time + 1000) {
    current_time = millis();
    the_button.read();
    if (the_button.released()) {
      module_solution = true;
      break;
    }
  }

  button_pressed = false;
}


void hold_and_wait() {
  // 
  current_time = millis();

  if (current_time >= last_action_time + 1000) {
    last_action_time = millis();
    minutes_ones = int(trunc((current_time/1000)/60)) % 10;
    seconds_tens = int(trunc(((int(trunc(current_time/1000))) % 60) / 10));
    seconds_ones = ((int(trunc(current_time/1000))) % 60) % 10;

    display_hold_and_wait();
  }

  if (the_button.released()) {
    button_pressed = false;

    if (light_strip_color == 2) {
      // If the light strip is blue
      if (minutes_ones == 4 || seconds_tens == 4 || seconds_ones == 4) {
        // And the timer has a 4 in any position.
        module_solution = true;
      }
    }
    else if (light_strip_color == 0) {
      // If the light strip is white
      if (minutes_ones == 5 || seconds_tens == 5 || seconds_ones == 5) {
        // And the timer has a 5 in any position.
        module_solution = true;
      }
    }
    else if (light_strip_color == 3) {
      // If the light strip is yellow
      if (minutes_ones == 1 || seconds_tens == 1 || seconds_ones == 1) {
        // And the timer has a 1 in any position.
        module_solution = true;
      }
    }
    else {
      // In any other case...
      if (minutes_ones == 5 || seconds_tens == 5 || seconds_ones == 5) {
        // ... if the timer has a 5 in any position.
        module_solution = true;
      }
    }
  }
}


void setup() {
  Serial.begin(9600);
  // randomSeed(analogRead(5));

  pinMode(STRIKE, OUTPUT);
  pinMode(SOLVED, OUTPUT);
  pinMode(MAIN_BUTTON, INPUT);

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display_1.begin(SSD1306_SWITCHCAPVCC,  0x3D);
  display_2.begin(SSD1306_SWITCHCAPVCC,  0x3C);
  display_1.display();
  display_2.display();
  delay(1000);

  // Clear the buffer.
  display_1.clearDisplay();
  display_2.clearDisplay();

  game_setup();

  module_leds.begin();
  module_leds.show(); // Initialize all pixels to 'off'
}

void loop() {
  while (!module_solution) {
    button_pressed = false;

    // Show the display and the button LEDs
    turn_off_lights();
    game_leds(b_color);
    delay(500);
    display_phase_1(button_text[button_game_text], shape, num_batteries);

    // While the button is not pressed
    while (!button_pressed) {
      the_button.read();

      if (the_button.pressed()) {
        // If the button is pressed, go to the next phase
        button_pressed = true;

        // Show the light strip and the counter screen if the soluton is to hold and wait.
        if (!game_solution) {
          current_time = millis();
          last_action_time = millis();
          minutes_ones = int(trunc((current_time/1000)/60)) % 10;
          seconds_tens = int(trunc(((int(trunc(current_time/1000))) % 60) / 10));
          seconds_ones = ((int(trunc(current_time/1000))) % 60) % 10;
        
          display_hold_and_wait();

          turn_off_lights();
          game_leds(b_color, true, light_strip_color);
        }
      }
    }

    // While the button is pressed
    while (button_pressed) {
      the_button.read();
      // Choose between hold and release, or hold and wait.
      if (game_solution) {
        hold_and_release();
      }
      else {
        hold_and_wait();
      }
    }

    // If after releasing the button the module is not solved, add a strike.
    if (!module_solution) {

      Serial.println("********** STRIKE **************");
      digitalWrite(STRIKE, HIGH);
      display_strike();
      delay(1000);
      digitalWrite(STRIKE, LOW);
    }
  }

  Serial.println("Sucess!!!");
  digitalWrite(SOLVED, HIGH);
  display_solved_module();
  
  while(1){}
}