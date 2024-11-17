#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>


#define OLED_MOSI     10
#define OLED_CLK      8
#define OLED_DC       7
#define OLED_CS       5
#define OLED_RST      9
#define BTN1 11
#define BTN2 12
#define BTN3 4
#define BTN4 13

// Button debouncing
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

Button button1      = { BTN1, HIGH, 0, 0 };
Button button2      = { BTN2, HIGH, 0, 0 };
Button button3      = { BTN3, HIGH, 0, 0 };
Button button4      = { BTN4, HIGH, 0, 0 };

// Create the OLED display
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64,OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

int column1[] = {0, 1, 2, 3, 4, 5, 6};
int column2[] = {7, 0, 6, 8, 9, 5, 10};
int column3[] = {11, 12, 8, 13, 14, 2, 9};
int column4[] = {15, 16, 17, 4, 13, 10, 18};
int column5[] = {19, 18, 17, 20, 16, 21, 22};
int column6[] = {15, 7, 23, 24, 19, 25, 26};
int *columns[] = {column1, column2, column3, column4, column5, column6};

static const unsigned char logo16_glcd_bmp [] PROGMEM = {
	// '1, 16x16px
	0x01, 0x80, 0x07, 0xe0, 0x0e, 0x70, 0x1c, 0x38, 0x18, 0x18, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 
	0x18, 0x18, 0x1c, 0x38, 0x0e, 0x70, 0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80
};

// Store the bitmaps in an array of pointers
const unsigned char *bitmaps[] = {logo16_glcd_bmp};
const int numBitmaps = sizeof(bitmaps) / sizeof(bitmaps[0]);

void selectRandomElements(int *array, int *selectedElements, int *selectedIndices, int arraySize, int numSelections) {
  for (int i = 0; i < numSelections; i++) {
    int index;
    bool unique;

    // Ensure unique selection
    do {
      unique = true;
      index = random(0, arraySize);

      // Check if the index has already been selected
      for (int j = 0; j < i; j++) {
        if (selectedIndices[j] == index) {
          unique = false;
          break;
        }
      }
    } while (!unique);

    // Store the selected element and its index
    selectedIndices[i] = index;
    selectedElements[i] = array[index];
  }
}

// Function to get the indices of the sorted values
void getSortedIndices(int *array, int *sortedIndices, int size) {
  // Initialize the sortedIndices array
  for (int i = 0; i < size; i++) {
    sortedIndices[i] = i;
  }

  // Sort the indices based on the values in the original array (using bubble sort for simplicity)
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (array[sortedIndices[j]] > array[sortedIndices[j + 1]]) {
        // Swap indices if the corresponding values are out of order
        int temp = sortedIndices[j];
        sortedIndices[j] = sortedIndices[j + 1];
        sortedIndices[j + 1] = temp;
      }
    }
  }
}

int img_order[4];
int solution[4];

void generateSolution() {
  int randomIndex = random(0, 6);
  int *selectedColumn = columns[randomIndex];
  Serial.print("Selected Column: ");
  int column_num = randomIndex + 1;
  Serial.println(column_num);
  int relative_idx[4];
  selectRandomElements(selectedColumn, img_order, relative_idx, 7, 4);
  Serial.print("Image order: ");
  for (int i = 0; i<4; i++) {
    Serial.print(img_order[i]);
    Serial.print(", ");
  } 
  Serial.println("");
  Serial.print("Relative index: ");
  for (int i = 0; i<4; i++) {
    Serial.print(relative_idx[i]);
    Serial.print(", ");
  } 
  Serial.println("");
  getSortedIndices(relative_idx,solution,4);

}

unsigned int X_COORDS [] = {24, 80, 24, 80};
unsigned int Y_COORDS [] = {8, 8, 32, 32};

#define IMG_HEIGHT 24
#define IMG_WIDTH  24

void initDisplay(int *img_order) {
  // Start OLED
  display.begin(0, true); // we dont use the i2c address but we will reset!
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);
  // Clear the buffer.
  display.clearDisplay();
  int idx = 0;
  for (uint8_t f = 0; f < 4; f++) {
      idx = img_order[f];
      display.drawBitmap(X_COORDS[f], Y_COORDS[f], bitmaps[idx], IMG_WIDTH, IMG_HEIGHT, SH110X_WHITE);
    }
    display.display();
    delay(200);


}
int counter = 0;
void readButtons() {
  int current;
  bool pressed = false;
  button1.read();
  button2.read();
  button3.read();
  button4.read();
  if (button1.pressed()) {
    current = 0;
    pressed = true;
  }
  if (button2.pressed()) {
    current = 1;
    pressed = true;
  }
  if (button3.pressed()) {
    current = 2;
    pressed = true;
  }
  if (button4.pressed()) {
    current = 3;
    pressed = true;
  }

  if (pressed & (current == solution[counter])) {
    pressed = false;
    counter++;
    Serial.print(counter);
    if (counter == 4) {
      digitalWrite(3, HIGH);
      while(1);
    }
  }
  else if (pressed & (current != solution[counter])) {
    pressed = false;
    counter = 0;
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
  }
}

void setup()   {

  pinMode(button1.pin,      INPUT);
  pinMode(button2.pin,      INPUT);
  pinMode(button3.pin,      INPUT);
  pinMode(button4.pin,      INPUT);
  pinMode(3, OUTPUT);
  pinMode(2, OUTPUT);

  Serial.begin(9600); delay(500);
  
  randomSeed(analogRead(0));  // Seed the random function
  //display.setContrast (0); // dim display
  for (int i = 0; i<4; i++) {
    img_order[i] = 0;
  } 
  initDisplay(img_order);
  generateSolution();

  Serial.print("Solution: ");
  for (int i = 0; i<4; i++) {
    int button_num = solution[i] + 1;
    Serial.print(button_num);
    Serial.print(", ");
  } 
  
  //testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);
}


void loop() {
  readButtons();
}