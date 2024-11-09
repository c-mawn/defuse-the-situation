#include <Arduino.h>
#include <TM1637Display.h>
#include <morse.h>

// Module definitions for communication with the main module.
#define STRIKE 8      // Pin to indicate a strike in-game
#define SOLVED 9    // Pin to indicate the module has been solved

// 7-segment display variables and definitions
#define CLK 3
#define DIO 4

TM1637Display display = TM1637Display(CLK, DIO);    // Create a display object of type TM1637Display

const uint8_t allON[] = {0xff, 0xff, 0xff, 0xff};   // Create an array that turns all segments ON
const uint8_t allOFF[] = {0x00, 0x00, 0x00, 0x00};  // Create an array that turns all segments OFF

// Input/Output Definitions
int freqSelectPin = A0;   // select the input pin for the potentiometer
int morseLedPin = 13;      // select the pin for the LED
int buttonPin = 5;    // Select the pin for the "send" button
int freqSelectValue = 0;  // variable to store the value coming from the sensor
long signal_frequency;

// Game definitions
String word_list[] = {"shell", "halls", "slick", "trick", "boxes", "leaks", "strobe", "bistro", "flick", "bombs", "break", "brick", "steak", "sting", "vector", "beats"};
bool module_solution = false;

int solution_word_id;
String solution_word;
int solution_frequency;

// Morse code sender definitions
float wpm_morse = 4.0;
LEDMorseSender morse_sender(morseLedPin, false, wpm_morse);    // The second parameter is for the WPM of the code

void setup() {
  pinMode(STRIKE, OUTPUT);
  pinMode(SOLVED, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  display.setBrightness(7);
  Serial.begin(9600);

  randomSeed(analogRead(5));        // Use the port A5 as a pseudo-random number to define de seed for the random function.
  // Otherwise it would use the same seed every time and get the same number the sketch runs
  solution_word_id = random(16);
  solution_word = word_list[solution_word_id];
  solution_frequency = (solution_word_id * 5) + 3505;

  morse_sender.setup();
  morse_sender.setMessage(solution_word);
}

void loop() {
  digitalWrite(SOLVED, LOW);
  // read the value from the sensor:
  while (!module_solution) {
    if (!morse_sender.continueSending()) {
      // Set the internal counters to the message's beginning.
      // Here, this results in repeating the message indefinitely.
      Serial.println("//////////////////////////////////////////");
      // delay(5000);
      morse_sender.startSending();
    }

    digitalWrite(STRIKE, LOW);
    Serial.println("**********************");
    Serial.println(solution_word);
    Serial.println(solution_frequency);
    freqSelectValue = analogRead(freqSelectPin);
    signal_frequency = (((long)freqSelectValue * 105)/1024) + 3500;
    signal_frequency = signal_frequency - (signal_frequency % 5);
    display.showNumberDec(signal_frequency);

    if (digitalRead(buttonPin)==LOW && signal_frequency) {
      if (signal_frequency == solution_frequency) {
        module_solution = true;
      }
      else {
        Serial.println("********** STRIKE **************");
        digitalWrite(STRIKE, HIGH);
        delay(500);
      }
    }
  }

  Serial.println("Sucess!!!");
  digitalWrite(SOLVED, HIGH);
  while(1){}
}