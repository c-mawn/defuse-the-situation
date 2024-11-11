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


// Create the OLED display
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64,OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

int column1[] = {0, 1, 2, 3, 4, 5, 6};
int column2[] = {7, 0, 6, 8, 9, 5, 10};
int column3[] = {11, 12, 8, 13, 14, 2, 9};
int column4[] = {15, 16, 17, 4, 13, 10, 18};
int column5[] = {19, 20, 17, 21, 16, 22, 23};
int column6[] = {15, 7, 24, 25, 19, 26, 27};
int *columns[] = {column1, column2, column3, column4, column5, column6};

static const unsigned char logo16_glcd_bmp [] PROGMEM = {
	// '1, 16x16px
	0x01, 0x80, 0x07, 0xe0, 0x0e, 0x70, 0x1c, 0x38, 0x18, 0x18, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 
	0x18, 0x18, 0x1c, 0x38, 0x0e, 0x70, 0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80
};

// Store the bitmaps in an array of pointers
const unsigned char *bitmaps[] = {logo16_glcd_bmp};
const int numBitmaps = sizeof(bitmaps) / sizeof(bitmaps[0]);
/*
void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  uint8_t icons[NUMFLAKES][3];

  // initialize
  for (uint8_t f = 0; f < NUMFLAKES; f++) {
    icons[f][XPOS] = random(display.width());
    icons[f][YPOS] = 0;
    icons[f][DELTAY] = random(5) + 1;

    Serial.print("x: ");
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(" y: ");
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(" dy: ");
    Serial.println(icons[f][DELTAY], DEC);
  }

  while (1) {
    // draw each icon
    for (uint8_t f = 0; f < NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SH110X_WHITE);
    }
    display.display();
    delay(200);

    // then erase it + move it
    for (uint8_t f = 0; f < NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SH110X_BLACK);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height()) {
        icons[f][XPOS] = random(display.width());
        icons[f][YPOS] = 0;
        icons[f][DELTAY] = random(5) + 1;
      }
    }
  }
}
*/
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

int *img_order[4];
int *relative_idx[4];

void generateSolution() {
  int randomIndex = random(0, 6);
  int *selectedColumn = columns[randomIndex];
  selectRandomElements(selectedColumn, *img_order, *relative_idx, 7, 4);

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
  for (uint8_t f = 0; f < 3; f++) {
      idx = img_order[f];
      display.drawBitmap(X_COORDS[f], Y_COORDS[f], bitmaps[idx], IMG_WIDTH, IMG_HEIGHT, SH110X_WHITE);
    }
    display.display();
    delay(200);


}

void setup()   {
  Serial.begin(9600); delay(500);

  randomSeed(analogRead(0));  // Seed the random function
  //display.setContrast (0); // dim display

  

  //testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);
}


void loop() {

}