#include <Arduino.h>
#include <FastLED.h>


// Module definitions for communication with the main module.
#define STRIKE 8      // Pin to indicate a strike in-game
#define SOLVED 9    // Pin to indicate the module has been solved

// Pin definitions for module output/input
#define BUTTON_1 3
#define BUTTON_2 4
#define BUTTON_3 5
#define BUTTON_4 6
#define LED_STRIP_PIN 7

// Definitions for LED strip
#define NUM_LEDS    10
#define BRIGHTNESS  10
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
int button_values[] = {{1, 2, 3, 4}, {1, 2, 3, 4}, {1, 2, 3, 4}, {1, 2, 3, 4}, {1, 2, 3, 4}};          // Number assigned to each button for all 5 stages
int stage_solutions[5][2];        // Solutions for each stage in the game. Also works as the history correct button press
// Solutions are written as: {position, label}, where label is the number assigned to each button

bool module_solution = false;     // Variable to indicate if the module is solved
int current_stage = 1;            // Current stage of the module (out of 5 stages)


void find_solution_position(int label, int stage_base1) {
  // Find the index of the value that solves a stage.
  // The index returned is in base 1.
  for (int solution_index = 0; solution_index < 4; solution_index++) {
    if (button_values[stage_base1 - 1][solution_index] == label) {
      return (solution_index + 1);
    }
  } 
}


void find_solutions() {
  // STAGE 1
  if (display_values[0] == 1) {
    // If the display is 1, press the button in the second position.
    stage_solutions[0] = {2, button_values[0][2 - 1]};
  }
  else if (display_values[0] == 2) {
    // If the display is 2, press the button in the second position.
    stage_solutions[0] = {2, button_values[0][2 - 1]};
  }
  else if (display_values[0] == 3) {
    // If the display is 3, press the button in the third position.
    stage_solutions[0] = {3, button_values[0][3 - 1]};
  }
  else if (display_values[0] == 4) {
    // If the display is 4, press the button in the fourth position.
    stage_solutions[0] = {4, button_values[0][4 - 1]};
  }

  // STAGE 2
  if (display_values[1] == 1) {
    // If the display is 1, press the button labeled "4".
    stage_solutions[1] = {find_solution_position(4, 2), 4};
  }
  else if (display_values[1] == 2) {
    // If the display is 2, press the button in the same position as you pressed in stage 1.
    stage_solutions[1] = {stage_solutions[0][0], button_values[1][stage_solutions[0][0]]};
  }
  else if (display_values[1] == 3) {
    // If the display is 3, press the button in the first position.
    stage_solutions[1] = {1, button_values[1][1 - 1]};
  }
  else if (display_values[1] == 4) {
    // If the display is 4, press the button in the same position as you pressed in stage 1.
    stage_solutions[1] = {stage_solutions[0][0], button_values[1][stage_solutions[0][0]]};
  }

  // STAGE 3
  if (display_values[2] == 1) {
    // If the display is 1, press the button with the same label you pressed in stage 2.
    stage_solutions[2] = {find_solution_position(stage_solutions[2-1][1], 3), stage_solutions[2-1][1]};
  }
  else if (display_values[2] == 2) {
    // If the display is 2, press the button with the same label you pressed in stage 1.
    stage_solutions[2] = {find_solution_position(stage_solutions[1-1][1], 3), stage_solutions[1-1][1]};
  }
  else if (display_values[2] == 3) {
    // If the display is 3, press the button in the third position.
    stage_solutions[2] = {3, button_values[2][3 - 1]};
  }
  else if (display_values[2] == 4) {
    // If the display is 4, press the button labeled "4".
    stage_solutions[2] = {find_solution_position(4, 3), 4};
  }

  // STAGE 4
  if (display_values[3] == 1) {
    // If the display is 1, press the button in the same position as you pressed in stage 1.
    stage_solutions[3] = {stage_solutions[1-1][0], button_values[4-1][stage_solutions[1-1][0]]};
  }
  else if (display_values[3] == 2) {
    // If the display is 2, press the button in the first position.
    stage_solutions[3] = {1, button_values[3][1 - 1]};
  }
  else if (display_values[3] == 3) {
    // If the display is 3, press the button in the same position as you pressed in stage 2.
    stage_solutions[3] = {stage_solutions[2-1][0], button_values[4-1][stage_solutions[2-1][0]]};
  }
  else if (display_values[3] == 4) {
    // If the display is 4, press the button in the same position as you pressed in stage 2
    stage_solutions[3] = {stage_solutions[2-1][0], button_values[4-1][stage_solutions[2-1][0]]};
  }

  // STAGE 5
  if (display_values[4] == 1) {
    // If the display is 1, press the button with the same label you pressed in stage 1.
    stage_solutions[4] = {find_solution_position(stage_solutions[1-1][1], 5), stage_solutions[1-1][1]};
  }
  else if (display_values[4] == 2) {
    // If the display is 2, press the button with the same label you pressed in stage 2.
    stage_solutions[4] = {find_solution_position(stage_solutions[2-1][1], 5), stage_solutions[2-1][1]};
  }
  else if (display_values[4] == 3) {
    // If the display is 3, press the button with the same label you pressed in stage 4.
    stage_solutions[4] = {find_solution_position(stage_solutions[4-1][1], 5), stage_solutions[4-1][1]};
  }
  else if (display_values[4] == 4) {
    // If the display is 4, press the button with the same label you pressed in stage 3.
    stage_solutions[4] = {find_solution_position(stage_solutions[3-1][1], 5), stage_solutions[3-1][1]};
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
  FastLED.addLeds<WS2812, LED_PIN, GRB>(memory_leds, NUM_LEDS);
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

  // Set display LED
  int display_color[] = colors[display_values[stage_base1]];
  memory_leds[0] = CRGB (display_color[0], display_color[1], display_color[2]);

  // Set button LEDs
  for (int i = 0; i < 4; i++) {
    int button_color[] = colors[button_values[stage_base1][i]];
    memory_leds[i + 1] = CRGB (button_color[0], button_color[1], button_color[2]);
  }

  // Set stage counter LEDs
  int counter_color[] = colors[5]   // Set the counter color to yellow
  for (int i = 1; i < stage_base1; i++) {
    memory_leds[i + 4] = CRGB (counter_color[0], counter_color[1], counter_color[2]);
  }

  // Display the stage
  FastLED.show();
  delay(40);
}

void setup() {
  pinMode(STRIKE, OUTPUT);
  pinMode(SOLVED, OUTPUT);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);

  initLights();
  initMemoryGame();
}

void loop() {
  while (!module_solution) {
    turn_off_lights();
    display_stage(current_stage);
    
    bool button_pressed = false;
    while (!button_pressed) {
      for (int button = 3; button <= 6; button++) {
        if (digitalRead(button) == LOW) {

          // If the button pressed is correct, advance to the next stage
          if ((button - 2) == stage_solutions[current_stage - 1][0]) {
            current_stage += 1;
          }
          // Else, return to stage 1 and add a strike
          else {
            current_stage = 1;
            digitalWrite(STRIKE, HIGH);
            delay(500);
          }

          // Break the loop once the button has been pressed
          button_pressed = true;
          break;
        };
      }
    }

    if (current_stage > 5){
      module_solution = true;
    }
  }

  digitalWrite(SOLVED, HIGH);
  while(1){}
}