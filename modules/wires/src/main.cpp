#include <Arduino.h>
#include <FastLED.h>

#define SW1 12
#define SW2 11
#define SW3 10
#define SW4 9
#define SW5 8
#define SW6 7
#define WIN_LED 6
#define LOSE_LED 5
#define LED_PIN     3
#define NUM_LEDS    24
#define BRIGHTNESS  10
#define LED_TYPE    WS2812B
//#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
//#define UPDATES_PER_SECOND 100
//CRGBPalette16 currentPalette;
//TBlendType    currentBlending;
//extern CRGBPalette16 myRedWhiteBluePalette;
//extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

int wire_indices[] = {11, 15, 19, 23}; // Array of possible values
int wire_index;
int colors[][3] = {
        {255, 0, 0},   // Red
        {255, 255, 255}, // White
        {255, 255, 0},   // Yellow
        {0, 0, 255},    // Blue
        {0, 255, 0}      // Green
    };
int R;
int G;
int B;
int state_big[6] = {};
int* state;
int num_wires;
int solution;
bool win = false;
const int switchPins[] = {SW1, SW2, SW3, SW4, SW5, SW6}; // List of switch pins
const int numSwitches = sizeof(switchPins) / sizeof(switchPins[0]);

void initLights() {
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(  BRIGHTNESS );
}

void initWires() {
    randomSeed(analogRead(0));
    int choose_wires = random(0, 4);
    num_wires = choose_wires + 3;
    wire_index = wire_indices[choose_wires];

    int counter = 0;
    for (int i = 0; i <= wire_index; i++) {
        if (i % 4 == 0) {
            int choose_color = random(0,5);
            state_big[counter] = choose_color;
            R = colors[choose_color][0];
            G = colors[choose_color][1];
            B = colors[choose_color][2];
            counter++;
        }
        leds[i] = CRGB ( R, G, B);
        FastLED.show();
        delay(40);
    }

    // Dynamically creates a correctly sized state array
    state = new int[num_wires];
      for (int i = 0; i < num_wires; i++) {
    state[i] = state_big[i];
  }
}

int countOccurrences(int arr[], int size, int target) {
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] == target) {
            count++;
        }
    }
    return count;
}

int findLastOccurrence(int arr[], int size, int target) {
    int lastIndex = -1;
    for (int i = 0; i < size; i++) {
        if (arr[i] == target) {
            lastIndex = i; // Update lastIndex to the current index
        }
    }
    return lastIndex + 1;
}

int calculateSolution() {
    switch (num_wires) {
    case 4:
        // statements
        if (countOccurrences(state, num_wires, 0) > 1){
            return findLastOccurrence(state, num_wires, 0);
        }
        else if (state[3] == 2 && countOccurrences(state, num_wires, 0) == 0){
            return 1;
        }
        else if (countOccurrences(state, num_wires, 3) == 1) {
            return 1;
        }
        else if (countOccurrences(state, num_wires, 2) > 1) {
            return 4;
        }
        else return 2;
    case 5:
        // statements
        if (state[4] == 4) {
            return 4;
        }
        else if (countOccurrences(state, num_wires, 0) == 1 && countOccurrences(state, num_wires, 2) > 1){
            return 1;
        }
        else if (countOccurrences(state, num_wires, 4) == 0) {
            return 2;
        }
        else return 1;
    case 6:
        // statements
        if (countOccurrences(state, num_wires, 2) == 0) {
            return 3;
        }
        else if (countOccurrences(state, num_wires, 2) == 1 && countOccurrences(state, num_wires, 1) > 1) {
            return 4;
        }
        else if (countOccurrences(state, num_wires, 0) == 0){
            return 6; 
        }
        else return 4;
    default:
        // statements
        if (countOccurrences(state, num_wires, 0) == 0) {
            return 2;
        }
        else if (state[2] == 1) {
            return 3;
        }
        else if (countOccurrences(state, num_wires, 3) > 1) {
            return findLastOccurrence(state, num_wires, 3); 
        }
        else return 3;
    }
 return 9;
}

int readSwitches() {
    for (int i = 0; i < numSwitches; i++) {
        if (digitalRead(switchPins[i]) == 0) {
            return i + 1; // Return 1 for SW1, 2 for SW2, etc.
        }
    }
    return 0; // Return 0 if no switch is pressed
}

void cutWire(int wire) {
    switch (wire) {
    case 1:
        for (int i=3; i>0; i--){
            leds[i] = CRGB::Black;
            FastLED.show();
            delay(40);
        }
        break;
    case 2:
        for (int i=7; i>3; i--){
            leds[i] = CRGB::Black;
            FastLED.show();
            delay(40);
        }
        break;
    case 3:
        for (int i=11; i>7; i--){
            leds[i] = CRGB::Black;
            FastLED.show();
            delay(40);
        }
        break;
    case 4:
        for (int i=15; i>11; i--){
            leds[i] = CRGB::Black;
            FastLED.show();
            delay(40);
        }
        break;
    case 5:
        for (int i=19; i>15; i--){
            leds[i] = CRGB::Black;
            FastLED.show();
            delay(40);
        }
        break;
    case 6:
        for (int i=23; i>19; i--){
            leds[i] = CRGB::Black;
            FastLED.show();
            delay(40);
        }
        break;
    default:
        // statements
        break;
    }
}

void setup() {
    Serial.begin(9600);
    initLights();
    initWires();
    Serial.println(num_wires);
    for (int i = 0; i<num_wires; i++){
        Serial.print(state[i]);
    }
    Serial.println("-------");
    solution = calculateSolution();
    Serial.println(solution);
    // Serial.println("______");
    // Serial.println(countOccurrences(state,num_wires,0));
    pinMode(WIN_LED, OUTPUT);
    pinMode(LOSE_LED, OUTPUT);
}

void loop() {
    while (!win){
        int cut_wire = readSwitches();
        //Serial.print(cut_wire);
        if (cut_wire != 0 && !win) {
            Serial.print(cut_wire);
            if (cut_wire == solution) {
                cutWire(cut_wire);
                win = true;
            }
            else if (cut_wire != solution) {
                while (readSwitches() != 0) {
                    digitalWrite(LOSE_LED, HIGH);
                }
                digitalWrite(LOSE_LED, LOW);
            }
        }
    }
    digitalWrite(WIN_LED, HIGH);
    
}