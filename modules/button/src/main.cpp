#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <FastLED.h>


// Module definitions for communication with the main module.
#define STRIKE        11    // Pin to indicate a strike in-game
#define SOLVED        12    // Pin to indicate the module has been solved

// Definitions for OLED display
#define OLED_MOSI     10
#define OLED_CLK      8
#define OLED_DC       7
#define OLED_CS       5
#define OLED_RST      9

// Create the OLED display
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);


// ***********  BITMAPS   ***********
// Small battery pixel art, 15x30px
const unsigned char small_battery [] PROGMEM = {
	0x1f, 0xf0, 0x1f, 0xf0, 0x1f, 0xf0, 0x1f, 0xf0, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 
	0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 
	0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 
	0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe
};

// Definitions for LED strip
#define LED_STRIP_PIN     4
#define NUM_BUTTON_LEDS   2       // 3
#define NUM_STRIP_LEDS    3       // 5
#define NUM_LEDS          (NUM_BUTTON_LEDS + NUM_STRIP_LEDS)
#define BRIGHTNESS        5
#define LED_TYPE          WS2812B
CRGB module_leds[NUM_LEDS];  //#define COLOR_ORDER GRB

// Game definitions and variables
#define MAIN_BUTTON       3

bool module_solution = false;     // Variable to indicate if the module is solved
int minutes_ones;
int seconds_tens;
int seconds_ones;
unsigned long current_time;
unsigned long last_action_time;
bool button_pressed;

const char button_text[4][9] = {"Detonate","Abort", "Hold", "Press"};
const int light_colors[][3] = {
        {255, 255, 255},  // 0 - White      - Light Strip & Button
        {255, 0, 0},      // 1 - Red        - Light Strip & Button
        {0, 0, 255},      // 2 - Blue       - Light Strip & Button
        {255, 255, 0},    // 3 - Yellow     - Light Strip & Button
        {255, 0, 255},    // 4 - Magenta    - Light Strip
        {0, 255, 0}       // 5 - Green      - Light Strip
    };
int b_color;              // Button color (Random number from 0 to 3 (inclusive)) from light_colors
int button_game_text;               // Button text (Random number from 0 to 3 (inclusive)) from button_texts
int light_strip_color;    // Light strip color (Random number from 0 to 5 (inclusive)) from light_colors
int num_batteries;        // Number of batteries (Up to 3 batteries, the number should be in base 1)
int shape;                // Shape index (Random number from 0 to 5 (inclusive))
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

void draw_centered_text(char display_text[], int text_size, int y_offset) {
  // Needs a clear display before the function, and a display after the function
  display.setTextSize(text_size);
  display.setTextColor(SH110X_WHITE);
  int16_t  x1, y1;
  uint16_t w, h;

  display.getTextBounds(display_text, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128/2)-(w/2), y_offset);
  display.println(display_text);
}


void draw_rectangle(int width, int height, int thickness, int x_0 = 0, int y_0 = 0) {
  for (int i = 0; i < thickness; i++) {
    display.drawRect(x_0 + i, y_0 + i, width - (2*i), height - (2*i), SH110X_WHITE);
  }
}


void draw_circle(int radius, int thickness, int x_0 = 0, int y_0 = 0) {
  display.fillCircle(x_0, y_0, radius, SH110X_WHITE);
  display.fillCircle(x_0, y_0, radius - (thickness), SH110X_BLACK);
}


void draw_triangle(int x_0, int y_0,int x_1, int y_1, int x_2, int y_2, int thickness, int x = 0, int y = 0) {
  display.fillTriangle(x_0 + x, y_0 + y, x_1 + x, y_1 + y, x_2 + x, y_2 + y, SH110X_WHITE);
  display.fillTriangle(
    (x_0 + x + 2*thickness), (y_0 + y - thickness),
    (x_1 + x), (y_1 + y + 2*thickness),
    (x_2 + x - 2*thickness), (y_2 + y - thickness),
    SH110X_BLACK);
}


void display_phase1(char word[], int shape_index, int n_batteries) {
  // Display configuration for the first stage of the module
  // (Before Pressing the button)

  display.clearDisplay();

  // Button word
  draw_rectangle(128, 24, 2);       // Rectangle enclosing the word
  draw_centered_text(word, 2, 5);   // Draw the word in the display
  draw_rectangle(65, 64 - 22, 2, 0, 22);    // Rectangle enclosing the shape

  // Shape information
  draw_rectangle(65, 64 - 22, 2, 0, 22);    // Rectangle enclosing the shape
  switch (shape_index) {
  // Choose which shape to display based on the game setup
    case 0:
      draw_triangle(6 - 15, 28, 6, 1, 6 + 15, 28, 4, 27, 28);        // Draw an empty triangle
      break;
    case 1:
      display.fillTriangle(6 + (27 - 15), 56, 6 + 27, 29, 6 + (27 + 15), 56, SH110X_WHITE);    // Filled triangle
      break;
    case 2:
      draw_circle(14, 4, 6 + 27, 28 + 15);                           // Draw an empty circle
      break;
    case 3:
      display.fillCircle(6 + 27, 28 + 15, 14, SH110X_WHITE);         // Draw a filled circle
      break;
    case 4:
      draw_rectangle(30, 30, 4, 6 + (54/2 - 15), 28);                // Draw an empty square
      break;
    case 5:
      display.fillRect(6 + (54/2 - 15), 28, 30, 30, SH110X_WHITE);   // Draw a filled square
      break;
    default:
      Serial.println("The shape index is not between boundaries");
      break;
  }

  // Battery information
  draw_rectangle(65, 64 - 22, 2, 63, 22);   // Rectangle enclosing the batteries
  display.drawBitmap(69 + 2*(4 + 15), 28, small_battery, 15, 30, SH110X_WHITE);   // Display the first battery
  if (n_batteries > 1) {
    display.drawBitmap(69 + (4 + 15), 28, small_battery, 15, 30, SH110X_WHITE);   // Display the second battery
  }
  if (n_batteries > 2) {
    display.drawBitmap(69, 28, small_battery, 15, 30, SH110X_WHITE);              // Display the third battery
  }

  display.display();
}


void display_hold_and_wait(int min_ones, int sec_tens, int sec_ones) {
  // Display configuration for the first stage of the module
  // (Before Pressing the button)

  display.clearDisplay();
  char m_ones[1] = {min_ones + 48};
  char s_tens[1] = {sec_tens + 48};
  char s_ones[1] = {sec_ones + 48};

  // Button word
  draw_centered_text(m_ones, 2, 3);   // Draw the minutes ones
  draw_centered_text(s_tens, 2, 23);   // Draw the seconds tens
  draw_centered_text(s_ones, 2, 43);   // Draw the seconds ones

  // BitMap Example
  // draw_rectangle(65, 64 - 22, 2, 63, 22);   // Rectangle enclosing the batteries
  // display.drawBitmap(69 + 2*(4 + 15), 28, small_battery, 15, 30, SH110X_WHITE);   // Display the first battery
  // if (n_batteries > 1) {
  //   display.drawBitmap(69 + (4 + 15), 28, small_battery, 15, 30, SH110X_WHITE);   // Display the second battery
  // }
  // if (n_batteries > 2) {
  //   display.drawBitmap(69, 28, small_battery, 15, 30, SH110X_WHITE);              // Display the third battery
  // }

  display.display();
}


void display_strike() {
  // 
  display.clearDisplay();

  // PLACE HOLDER FOR REAL STRIKE DISPLAY ************************************
  display.drawBitmap(69 + 2*(4 + 15), 28, small_battery, 15, 30, SH110X_WHITE);   // Display the first battery

  display.display();
}


void display_solved_module() {
  // 
  display.clearDisplay();

  // PLACE HOLDER FOR REAL SOLVED DISPLAY *********************
  display.drawBitmap(3, 3, small_battery, 15, 30, SH110X_WHITE);   // Display the first battery
  
  display.display();
}


// LED strip functions

void initLights() {
  delay(3000); // power-up safety delay
  FastLED.addLeds<WS2812, LED_STRIP_PIN, GRB>(module_leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}


void turn_off_lights() {
  for (int i = 0; i > NUM_LEDS; i++){
    module_leds[i] = CRGB::Black;
    FastLED.show();
    delay(40);
  }
}


void game_leds(int button_color, bool light_strip = false, int light_strip_color = 0) {
  // Turns on the LEDs for the button and the light strip

  // Set button LEDs
  for (int i = 0; i < NUM_BUTTON_LEDS; i++) {
    module_leds[i] = CRGB (
      light_colors[button_color][0],            // Assign to the LED the button color
      light_colors[button_color][1],
      light_colors[button_color][2]);
  }

  // Set light strip LEDs
  for (int i = 0; i < NUM_STRIP_LEDS; i++) {
    if (light_strip == true) {
      module_leds[i + NUM_STRIP_LEDS] = CRGB (
        light_colors[light_strip_color][0],     // Assign to LEDs the light strip color
        light_colors[light_strip_color][1],
        light_colors[light_strip_color][2]);
    }
    else {
      module_leds[i + NUM_STRIP_LEDS] = CRGB::Black;
    }
  }

  // Show the color of the button and the light strip LEDs
  FastLED.show();
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
  // 
  last_action_time = millis();
  current_time = millis();

  while (current_time < last_action_time + 1000) {
    current_time = millis();
    if (digitalRead(MAIN_BUTTON) != LOW) {
      module_solution = true;
      break;
    }
  }

  button_pressed = false;
}


void hold_and_wait() {
  // 
  current_time = millis();

  if (current_time < last_action_time + 1000) {
    last_action_time = millis();
    minutes_ones = int(trunc((current_time/1000)/60)) % 10;
    seconds_tens = int(trunc((int(trunc(current_time/1000)) % 60) / 10));
    seconds_ones = (int(trunc(current_time/1000)) % 60) % 10;

    display_hold_and_wait(minutes_ones, seconds_tens, seconds_ones);
  }

  if (digitalRead(MAIN_BUTTON) != LOW) {
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
      if (minutes_ones == 1 || seconds_tens == 1 || seconds_ones == 1) {
        // And the timer has a 1 in any position.
        module_solution = true;
      }
    }
    else if (light_strip_color == 3) {
      // If the light strip is yellow
      if (minutes_ones == 5 || seconds_tens == 5 || seconds_ones == 5) {
        // And the timer has a 5 in any position.
        module_solution = true;
      }
    }
    else {
      // In any other case...
      if (minutes_ones == 1 || seconds_tens == 1 || seconds_ones == 1) {
        // ... if the timer has a 1 in any position.
        module_solution = true;
      }
    }
  }
}


void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(5));

  pinMode(STRIKE, OUTPUT);
  pinMode(SOLVED, OUTPUT);
  pinMode(MAIN_BUTTON, INPUT_PULLUP);

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.begin(0, true);
  display.display();
  delay(1000);
  Serial.println("Screen started");

  // Clear the buffer.
  display.clearDisplay();

  game_setup();

  initLights();
  Serial.println("LIGHTS STARTED ************");
}

void loop() {
  while (!module_solution) {
    button_pressed = false;

    // Show the display and the button LEDs
    turn_off_lights();
    game_leds(b_color);
    delay(500);
    display_phase1(button_text[button_game_text], shape, num_batteries);

    // While the button is not pressed
    while (!button_pressed) {
      Serial.println(b_color);
      if (digitalRead(MAIN_BUTTON) == LOW) {
        // If the button is pressed, go to the next phase
        button_pressed = true;

        last_action_time = millis();
        minutes_ones = int(trunc((current_time/1000)/60)) % 10;
        seconds_tens = int(trunc((int(trunc(current_time/1000)) % 60) / 10));
        seconds_ones = (int(trunc(current_time/1000)) % 60) % 10;
        
        display_hold_and_wait(minutes_ones, seconds_tens, seconds_ones);

        // Show the light strip if the soluton is to hold and wait.
        if (!game_solution) {
          turn_off_lights();
          game_leds(b_color, true, light_strip_color);
        }
      }
    }

    // While the button is pressed
    while (button_pressed) {
      Serial.println(light_strip_color);
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

      // ........... Add a screen for the strike to avoid confusion.

      Serial.println("********** STRIKE **************");
      digitalWrite(STRIKE, HIGH);
      delay(500);
      digitalWrite(STRIKE, LOW);
      display_strike();
      delay(2000);      // Wait 2 seconds to let the player lift their hands from the button.
    }
  }

  Serial.println("Sucess!!!");
  digitalWrite(SOLVED, HIGH);
  display_solved_module();
  
  while(1){}
}