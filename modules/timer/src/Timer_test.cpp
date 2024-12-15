/*
 * For the timer module, Connect Display pins: 
 CLK to A0
 DIO to 10
 VCC to +5V
 GND to GND
 Button Solved to 2
 Memory Solved to 3
 Wires Solved to 4
 Morse Solved to 5
 Keypad Solved to 6
 All Strikes to 7
 Strikes 1,2,3 to 11,12,13
 */
#include <Arduino.h>
#include <AceTMI.h>
#include <AceSegment.h>

using ace_tmi::SimpleTmi1637Interface;
using ace_segment::Tm1637Module;

/*
 * Defining Pins for the timer
 */
const uint8_t CLK_PIN = A0;
const uint8_t DIO_PIN = 10;
const uint8_t NUM_DIGITS = 4;
const uint8_t SOLVE_PIN_BUTTON = 2;
const uint8_t SOLVE_PIN_MEMORY = 3;
const uint8_t SOLVE_PIN_WIRES = 4;
const uint8_t SOLVE_PIN_MORSE = 5;
const uint8_t SOLVE_PIN_KEYPAD = 6;
const uint8_t STRIKE_PIN = A1;
const int STRIKE_THRESH = 100;
const uint8_t STRIKE1 = 11;
const uint8_t STRIKE2 = 12;
const uint8_t STRIKE3 = 13;


/*
 * Timer initalization values
 */
const unsigned long countdownTime = 300000; // 5 minutes in milliseconds
unsigned long startTime;
unsigned long elapsedTime;
unsigned long current_time;
unsigned long last_time;

/*
 * Delay time for compensation in the electronics
 */
const uint8_t DELAY_MICROS = 100;

using TmiInterface = SimpleTmi1637Interface;
TmiInterface tmiInterface(DIO_PIN, CLK_PIN, DELAY_MICROS);
Tm1637Module<TmiInterface, NUM_DIGITS> ledModule(tmiInterface);

// LED segment patterns.
const uint8_t NUM_PATTERNS = 10;
const uint8_t PATTERNS[NUM_PATTERNS] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9
};

// Full Game variables
int strikes = 0;
int prev_strike_state = 0;
int current_strike_state = 0;
//int modules_solved = 0;

void explode(){
  ledModule.setPatternAt(0, 0b01000000);
  ledModule.setPatternAt(1, 0b01000000);
  ledModule.setPatternAt(2, 0b01000000);
  ledModule.setPatternAt(3, 0b01000000);
  Serial.print("boom");
  while(true){}
}

void defuse(){
  ledModule.setPatternAt(0, 0b01111111);
  ledModule.setPatternAt(1, 0b01111111);
  ledModule.setPatternAt(2, 0b01111111);
  ledModule.setPatternAt(3, 0b01111111);
  Serial.print("Yippee");
  while(true){}
}

void strike_led(){
  if(strikes == 1){
    digitalWrite(STRIKE1, HIGH);
  }
  else if (strikes == 2){
    digitalWrite(STRIKE2, HIGH);
  }
  else if(strikes == 3){
    digitalWrite(STRIKE3, HIGH);
    explode();
  }
  
}

void check_modules(){
  if (analogRead(STRIKE_PIN) >= STRIKE_THRESH){
    prev_strike_state = current_strike_state;
    current_strike_state = 1;
  }
  else {
    prev_strike_state = current_strike_state;
    current_strike_state = 0;
  }

  // Check module status
  if(digitalRead(SOLVE_PIN_BUTTON) == HIGH && digitalRead(SOLVE_PIN_KEYPAD) == HIGH && digitalRead(SOLVE_PIN_WIRES) == HIGH && digitalRead(SOLVE_PIN_MEMORY) == HIGH && digitalRead(SOLVE_PIN_MORSE) == HIGH){
    Serial.println("solved");
    defuse();
  }

  if(prev_strike_state == 0 && current_strike_state == 1){
    Serial.print("Strike");
    Serial.println(strikes);
    strikes++;
    strike_led();
  }
}

void setup(){
  Serial.begin(9600);
  delay(1000); // gives time to start up
  startTime = millis();
  tmiInterface.begin();
  ledModule.begin();

  pinMode(SOLVE_PIN_BUTTON, INPUT);
  pinMode(SOLVE_PIN_KEYPAD, INPUT);
  pinMode(SOLVE_PIN_WIRES, INPUT);
  pinMode(SOLVE_PIN_MORSE, INPUT);
  pinMode(SOLVE_PIN_MEMORY, INPUT);

  pinMode(STRIKE_PIN, INPUT);

  pinMode(STRIKE1, OUTPUT);
  pinMode(STRIKE2, OUTPUT);
  pinMode(STRIKE3, OUTPUT);
  
}

void loop(){
  
  Serial.println(analogRead(STRIKE_PIN));
  
  check_modules();
  
  // Calculate elapsed time
  elapsedTime = millis() - startTime;
  unsigned long remainingTime = countdownTime - elapsedTime;  

  // Display countdown value
  int minutes = remainingTime / 60000;
  int seconds = (remainingTime % 60000) / 1000;

  // Split minutes and seconds into separate digits
  int min1 = minutes / 10; // Tens place of minutes
  int min2 = minutes % 10; // Ones place of minutes
  int sec1 = seconds / 10; // Tens place of seconds
  int sec2 = seconds % 10; // Ones place of seconds

  current_time = millis();

  if (current_time >= last_time + 1000) {
    last_time = current_time;
    // Print as four separate integers
    //ledModule.setPatternAt(0, PATTERNS[0]);
    ledModule.setPatternAt(1, PATTERNS[min2]);
    ledModule.setPatternAt(2, PATTERNS[sec1]);
    ledModule.setPatternAt(3, PATTERNS[sec2]);
    //ledModule.setPatternAt(
    ledModule.setBrightness(2);
    ledModule.flush();
  }

  if(min1 + min2 + sec1 + sec2 == 0){
    explode();
  }


  
}


