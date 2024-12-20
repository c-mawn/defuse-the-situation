#include <Arduino.h>
#include <FastLED.h>

// Button debounce code
const uint8_t DEBOUNCE_DELAY = 10; // in milliseconds

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
            // don't forget that the read pin is pulled-up
            bool pressed = reading == LOW;
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
#define STRIKE 8      // Pin to indicate a strike in-game
#define SOLVED 9    // Pin to indicate the module has been solved

// Pin definitions for module output/input
#define LED_STRIP_PIN   3
#define BUTTON_1        4
#define BUTTON_2        5
#define BUTTON_3        6
#define BUTTON_4        7

Button button_n1      = { BUTTON_1, HIGH, 0, 0 };
Button button_n2      = { BUTTON_2, HIGH, 0, 0 };
Button button_n3      = { BUTTON_3, HIGH, 0, 0 };
Button button_n4      = { BUTTON_4, HIGH, 0, 0 };

// Definitions for LED strip
#define NUM_LEDS    19
#define BRIGHTNESS  30
#define LED_TYPE    WS2812B
CRGB memory_leds[NUM_LEDS];  //#define COLOR_ORDER GRB
const int colors[][3] = {
        {255, 255, 255},  // White
        {255, 0, 0},      // Red - Corresponding to #1 in the game
        {0, 255, 0},      // Green - Corresponding to #2 in the game
        {0, 0, 255},      // Blue - Corresponding to #3 in the game
        {255, 0, 255},    // Magenta - Corresponding to #4 in the game
        {255, 255, 0}     // Yellow
    };

// GAME LOGIC DEFINTIONS AND VARIABLES
// Assigned number to color definitions
#define RED     1
#define GREEN   2
#define BLUE    3
#define MAGENTA 4

int display_values[5];            // Values shown in the display for all 5 stages
int button_values[5][4] = {{1, 2, 3, 4}, {1, 2, 3, 4}, {1, 2, 3, 4}, {1, 2, 3, 4}, {1, 2, 3, 4}};          // Number assigned to each button for all 5 stages
int stage_solutions[5][2];        // Solutions for each stage in the game. Also works as the history correct button press
// Solutions are written as: {position, label}, where label is the number assigned to each button

bool module_solution = false;     // Variable to indicate if the module is solved
int current_stage = 1;            // Current stage of the module (out of 5 stages)


int find_solution_position(int label, int stage_base1) {
  // Find the index of the value that solves a stage.
  // The index returned is in base 1.
  for (int solution_index = 0; solution_index < 4; solution_index++) {
    if (button_values[stage_base1 - 1][solution_index] == label) {
      solution_index += 1;
      Serial.println(solution_index);
      return solution_index;
    }
  }
}


void find_solutions() {
  // STAGE 1
  if (display_values[0] == 1) {
    // If the display is 1, press the button in the second position.
    stage_solutions[0][0] = 2;
    stage_solutions[0][1] = button_values[0][2 - 1];
  }
  else if (display_values[0] == 2) {
    // If the display is 2, press the button in the second position.
    stage_solutions[0][0] = 2,
    stage_solutions[0][1] = button_values[0][2 - 1];
  }
  else if (display_values[0] == 3) {
    // If the display is 3, press the button in the third position.
    stage_solutions[0][0] = 3;
    stage_solutions[0][1] = button_values[0][3 - 1];
  }
  else if (display_values[0] == 4) {
    // If the display is 4, press the button in the fourth position.
    stage_solutions[0][0] = 4;
    stage_solutions[0][1] = button_values[0][4 - 1];
  }

  // STAGE 2
  if (display_values[1] == 1) {
    // If the display is 1, press the button labeled "4".
    stage_solutions[1][0] = find_solution_position(4, 2);
    stage_solutions[1][1] = 4;
  }
  else if (display_values[1] == 2) {
    // If the display is 2, press the button in the same position as you pressed in stage 1.
    stage_solutions[1][0] = stage_solutions[0][0];
    stage_solutions[1][1] = button_values[1][stage_solutions[0][0] - 1];
    // Decrease 1 for the button to get since they are in base 1 in stage_solutions
  }
  else if (display_values[1] == 3) {
    // If the display is 3, press the button in the first position.
    stage_solutions[1][0] = 1;
    stage_solutions[1][1] = button_values[1][1 - 1];
  }
  else if (display_values[1] == 4) {
    // If the display is 4, press the button in the same position as you pressed in stage 1.
    stage_solutions[1][0] = stage_solutions[0][0];
    stage_solutions[1][1] = button_values[1][stage_solutions[0][0] - 1];
    // Decrease 1 for the button to get since they are in base 1 in stage_solutions
  }

  // STAGE 3
  if (display_values[2] == 1) {
    // If the display is 1, press the button with the same label you pressed in stage 2.
    stage_solutions[2][0] = find_solution_position(stage_solutions[2-1][1], 3);
    stage_solutions[2][1] = stage_solutions[2-1][1];
  }
  else if (display_values[2] == 2) {
    // If the display is 2, press the button with the same label you pressed in stage 1.
    stage_solutions[2][0] = find_solution_position(stage_solutions[1-1][1], 3);
    stage_solutions[2][1] = stage_solutions[1-1][1];
  }
  else if (display_values[2] == 3) {
    // If the display is 3, press the button in the third position.
    stage_solutions[2][0] = 3;
    stage_solutions[2][1] = button_values[2][3 - 1];
  }
  else if (display_values[2] == 4) {
    // If the display is 4, press the button labeled "4".
    stage_solutions[2][0] = find_solution_position(4, 3);
    stage_solutions[2][1] = 4;
  }

  // STAGE 4
  if (display_values[3] == 1) {
    // If the display is 1, press the button in the same position as you pressed in stage 1.
    stage_solutions[3][0] = stage_solutions[1-1][0];
    stage_solutions[3][1] = button_values[4-1][stage_solutions[1-1][0] - 1];
    // Decrease 1 for the button to get since they are in base 1 in stage_solutions
  }
  else if (display_values[3] == 2) {
    // If the display is 2, press the button in the first position.
    stage_solutions[3][0] = 1;
    stage_solutions[3][1] = button_values[3][1 - 1];
  }
  else if (display_values[3] == 3) {
    // If the display is 3, press the button in the same position as you pressed in stage 2.
    stage_solutions[3][0] = stage_solutions[2-1][0];
    stage_solutions[3][1] = button_values[4-1][stage_solutions[2-1][0] - 1];
    // Decrease 1 for the button to get since they are in base 1 in stage_solutions
  }
  else if (display_values[3] == 4) {
    // If the display is 4, press the button in the same position as you pressed in stage 2
    stage_solutions[3][0] = stage_solutions[2-1][0];
    stage_solutions[3][1] = button_values[4-1][stage_solutions[2-1][0] - 1];
    // Decrease 1 for the button to get since they are in base 1 in stage_solutions
  }

  // STAGE 5
  if (display_values[4] == 1) {
    // If the display is 1, press the button with the same label you pressed in stage 1.
    stage_solutions[4][0] = find_solution_position(stage_solutions[1-1][1], 5);
    stage_solutions[4][1] = stage_solutions[1-1][1];
  }
  else if (display_values[4] == 2) {
    // If the display is 2, press the button with the same label you pressed in stage 2.
    stage_solutions[4][0] = find_solution_position(stage_solutions[2-1][1], 5);
    stage_solutions[4][1] = stage_solutions[2-1][1];
  }
  else if (display_values[4] == 3) {
    // If the display is 3, press the button with the same label you pressed in stage 4.
    stage_solutions[4][0] = find_solution_position(stage_solutions[4-1][1], 5);
    stage_solutions[4][1] = stage_solutions[4-1][1];
  }
  else if (display_values[4] == 4) {
    // If the display is 4, press the button with the same label you pressed in stage 3.
    stage_solutions[4][0] = find_solution_position(stage_solutions[3-1][1], 5);
    stage_solutions[4][1] = stage_solutions[3-1][1];
  }
}


void initMemoryGame() {
  randomSeed(analogRead(5));

  for (int i = 0; i < 5; i++) {
    display_values[i] = random(1, 5);   // Choose a value from 1 to 4 for each stage
    
    for (int a = (4 - 1); a > 0; a--) {
      // Shuffle the 4 values for each stage using the Fisher-Yates algorithm
      int b = random(a + 1);
      int hold = button_values[i][a];
      button_values[i][a] = button_values[i][b];
      button_values[i][b] = hold;
    }
  }
  find_solutions();   // Find the solution of the game with the random setup
}


void initLights() {
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<WS2812, LED_STRIP_PIN, GRB>(memory_leds, NUM_LEDS);
  FastLED.setBrightness(  BRIGHTNESS );
}


void turn_off_lights() {
  for (int i = 0; i > NUM_LEDS; i++){
    memory_leds[i] = CRGB::Black;
    FastLED.show();
    delay(40);
  }
}


void display_stage(int stage_base1) {
  // Displays the current game stage of the game in the LED strip.
  // Does not display the game after winning the game.

  int stage_base0 = stage_base1 - 1;            // Convert the stage into a base 0 index to use in arrays

  // Set display LED
  for (int i = 0; i < 10; i++) {
    memory_leds[i + 4] = CRGB (
      colors[display_values[stage_base0]][0], // Assign to the LED the display color in the current stage
      colors[display_values[stage_base0]][1],
      colors[display_values[stage_base0]][2]);
  }

  // Set button LEDs
  for (int i = 0; i < 4; i++) {
    memory_leds[i] = CRGB (
      colors[button_values[stage_base0][i]][0], // Assign to the LED the button color in the current stage
      colors[button_values[stage_base0][i]][1],
      colors[button_values[stage_base0][i]][2]);
  }

  // Set stage counter LEDs
  for (int i = 1; i < stage_base1; i++) {
    // Assign to LEDs for the stage the color yellow (index 5 in the constant "colors")
    memory_leds[i + 13] = CRGB (colors[5][0], colors[5][1], colors[5][2]);
  }
  for (int i = stage_base1; i <= 5; i++){
    // Turn off the rest of the LEDs for the not completed stages
    memory_leds[i + 13] = CRGB::Black;
  }

  // Display the stage
  FastLED.show();
  delay(40);
}

void display_final_stage() {
  // Displays the winning game status after completing the module

  // Change the display and button LEDs (0 to 4) to white showing the module is completed
  for (int i = 0; i < 14; i++) {
    // Index 0 in the constant colors has the RGB code for white
    memory_leds[i] = CRGB (colors[0][0], colors[0][1], colors[0][2]);
  }

  // Display all the stage LEDs in yellow to show completition of all modules
  for (int i = 1; i <= 5; i++) {
    // Index 5 in the constant colors has the RGB code for yellow
    memory_leds[i + 13] = CRGB (colors[5][0], colors[5][1], colors[5][2]);
  }

  // Display the stage
  FastLED.show();
  delay(40);
}

void setup() {
  Serial.begin(9600);
  pinMode(STRIKE, OUTPUT);
  pinMode(SOLVED, OUTPUT);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);

  initLights();
  initMemoryGame();

  Serial.print("Testing: ");
  Serial.print(stage_solutions[2-1][0]);
  Serial.print(", ");
  Serial.println(button_values[4-1][stage_solutions[2-1][0] - 1]);

  Serial.print("Button Values. 1: (");
  Serial.print(button_values[0][0]);
  Serial.print(", ");
  Serial.print(button_values[0][1]);
  Serial.print(", ");
  Serial.print(button_values[0][2]);
  Serial.print(", ");
  Serial.print(button_values[0][3]);
  Serial.print(")");

  Serial.print(" 2: (");
  Serial.print(button_values[1][0]);
  Serial.print(", ");
  Serial.print(button_values[1][1]);
  Serial.print(", ");
  Serial.print(button_values[1][2]);
  Serial.print(", ");
  Serial.print(button_values[1][3]);
  Serial.print(")");

  Serial.print(" 3: (");
  Serial.print(button_values[2][0]);
  Serial.print(", ");
  Serial.print(button_values[2][1]);
  Serial.print(", ");
  Serial.print(button_values[2][2]);
  Serial.print(", ");
  Serial.print(button_values[2][3]);
  Serial.print(")");

  Serial.print(" 4: (");
  Serial.print(button_values[3][0]);
  Serial.print(", ");
  Serial.print(button_values[3][1]);
  Serial.print(", ");
  Serial.print(button_values[3][2]);
  Serial.print(", ");
  Serial.print(button_values[3][3]);
  Serial.print(")");

  Serial.print(" 5: (");
  Serial.print(button_values[4][0]);
  Serial.print(", ");
  Serial.print(button_values[4][1]);
  Serial.print(", ");
  Serial.print(button_values[4][2]);
  Serial.print(", ");
  Serial.print(button_values[4][3]);
  Serial.println(")");




  Serial.print("Display: ");
  Serial.print(display_values[0]);
  Serial.print(", ");
  Serial.print(display_values[1]);
  Serial.print(", ");
  Serial.print(display_values[2]);
  Serial.print(", ");
  Serial.print(display_values[3]);
  Serial.print(", ");
  Serial.println(display_values[4]);

  Serial.print("Solutions: (");
  Serial.print(stage_solutions[0][0]);
  Serial.print(", ");
  Serial.print(stage_solutions[0][1]);
  Serial.print("), (");

  Serial.print(stage_solutions[1][0]);
  Serial.print(", ");
  Serial.print(stage_solutions[1][1]);
  Serial.print("), (");

  Serial.print(stage_solutions[2][0]);
  Serial.print(", ");
  Serial.print(stage_solutions[2][1]);
  Serial.print("), (");

  Serial.print(stage_solutions[3][0]);
  Serial.print(", ");
  Serial.print(stage_solutions[3][1]);
  Serial.print("), (");

  Serial.print(stage_solutions[4][0]);
  Serial.print(", ");
  Serial.print(stage_solutions[4][1]);
  Serial.println(")");
}

void loop() {
  while (!module_solution) {
    // Read all the buttons
    button_n1.read();
    button_n2.read();
    button_n3.read();
    button_n4.read();
    
    turn_off_lights();
    display_stage(current_stage);
    
    bool button_pressed = false;
    while (!button_pressed) {
      // Read all the buttons
      button_n1.read();
      button_n2.read();
      button_n3.read();
      button_n4.read();

      // Count how many buttons are pressed at the same time
      byte button_presses = 0;
      // Get the last button pressed
      byte current_button = 0;

      // Check if a button is pressed
      if (button_n1.pressed()) {
        current_button = 1;
        button_presses += 1;
      }
      if (button_n2.pressed()) {
        current_button = 2;
        button_presses += 1;
      }
      if (button_n3.pressed()) {
        current_button = 3;
        button_presses += 1;
      }
      if (button_n4.pressed()) {
        current_button = 4;
        button_presses += 1;
      }
      
      // If only one button is pressed and it is the correct button, advance to the next stage
      if (button_presses == 1 && current_button == stage_solutions[current_stage - 1][0]) {
        current_stage += 1;
        delay(500);
        button_pressed = true;    // Break the loop once the button is pressed
      }
      // Else, If an incorrect button is pressed, or more than one button is pressed, return to stage 1 and add a strike.
      else if (button_presses) {
        current_stage = 1;
        digitalWrite(STRIKE, HIGH);
        delay(1000);
        digitalWrite(STRIKE, LOW);
        button_pressed = true;    // Break the loop once the button is pressed
      }
    }

    if (current_stage > 5){
      module_solution = true;
    }
  }

  digitalWrite(SOLVED, HIGH);
  display_final_stage();
  while(1){}
}