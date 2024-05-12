/**
 * Main code for on-field systems (audio, hit detection, display) for
 * ECE484 final project, Cardboard Pinball.
 * 
 * Author: Chris Lallo, Blake Rile
 * Date: 5/6/2024
 */

 /**
  * NOTES
  * Song list:
  * 
  * 0001 - Crowd cheering
  * 0002 - Sad crowd
  * 0003 - Glasglow Celtic
  * 0004 - Du Du Du
  * 0005 - Celtic Wave
  * 0006 - Goal scream
  */

// INCLUDES
#include "DFRobotDFPlayerMini.h"
#include "SoftwareSerial.h"
#include <LiquidCrystal_I2C.h>

// Setup objects
LiquidCrystal_I2C lcd(0x27,  16, 2);
SoftwareSerial mySerial(10, 11); // RX, TX
DFRobotDFPlayerMini DFMini;


// CONSTANTS
/////////////////////////////////////////////////////////
const int GOAL_INTERRUPT_PIN = 3;
const int GOALIE_INTERRUPT_PIN = 2;
const int DEFENDER_INTERRUPT_PIN = 18;

const int GOALIE_SERVO_PIN = 52; // Digital output for flipping up goalie
const int DEFENDER_SERVO_PIN = 53; // Digital output for flipping up defender

const int GOALIE_FLIPUP_VALUE = 1; // Flipup when goals >= value
const int DEFENDER_FLIPUP_VALUE = 3;

const int MINIMUM_POINTS = -200; // Player loses when points <= value
const unsigned long DEBOUNCE_DELAY = 100; // Debounce for hit detection
/////////////////////////////////////////////////////////

// GLOBALS
/////////////////////////////////////////////////////////
volatile bool goalHit = false;
volatile bool goalieHit = false;
volatile bool defenderHit = false;

volatile unsigned long lastGoalTime = 0;
volatile unsigned long lastGoalieTime = 0;
volatile unsigned long lastDefenderTime = 0;
volatile unsigned long lastSongTick = 0;
volatile unsigned int currentSong = 3;

volatile int points = 0;
volatile int goals = 0;

volatile bool playerLoss = false; // Goes high when MINIMUM_POINTS reached
/////////////////////////////////////////////////////////

// Core
/////////////////////////////////////////////////////////
void setup() {
  // Setup serial comms
  Serial.begin(9600);
  Serial.println("Serial comms online");

  // Generate random seed through analog noise
  randomSeed(analogRead(0));
  
  // Setup audio
  mySerial.begin(9600);
  if (!DFMini.begin(mySerial)) {
    Serial.println("Failed to initialize communication with DFPlayer");
    while (true) {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  DFMini.volume(30);  //Set volume value. From 0 to 30
  Serial.println("DFPlayer online");
  
  // Setup interrupts for hit detection
  pinMode(GOAL_INTERRUPT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(GOAL_INTERRUPT_PIN), goalReaction, RISING);

  pinMode(GOALIE_INTERRUPT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(GOALIE_INTERRUPT_PIN), goalieReaction, RISING);

  pinMode(DEFENDER_INTERRUPT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(DEFENDER_INTERRUPT_PIN), defenderReaction, RISING);

  // Setup digital outputs for activating servos (goalie and defender)
  pinMode(GOALIE_SERVO_PIN, OUTPUT);
  pinMode(DEFENDER_SERVO_PIN, OUTPUT);

  digitalWrite(GOALIE_SERVO_PIN, LOW);
  digitalWrite(DEFENDER_SERVO_PIN, LOW);
  
  // Setup LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("POINTS");
  lcd.setCursor(11, 0);
  lcd.print("GOALS");

  lcd.setCursor(1, 1);
  lcd.print(0);
  lcd.setCursor(12, 1);
  lcd.print(0);
}

void loop() {
  // If player loses, constantly play losing sound
  if (points <= MINIMUM_POINTS) {
    playerLoss = true;
  }
  if (playerLoss) {
    Serial.println("Player lost. Playing losing sound");
    DFMini.play(2);
    delay(3000);
  } else {
     // Look for triggered interrupts
    bool hitDetected = checkForHits();
    // Handle servo flipups
    if (goals >= DEFENDER_FLIPUP_VALUE) {
//      digitalWrite(GOALIE_SERVO_PIN, HIGH);
      digitalWrite(DEFENDER_SERVO_PIN, HIGH);
    } else if (goals >= GOALIE_FLIPUP_VALUE) {
      digitalWrite(GOALIE_SERVO_PIN, HIGH);
    }
    if (!hitDetected) {
      Serial.println("Scheduling audio...");
      scheduleAudio();
    }
  }
}

/**
 * 
 */
void scheduleAudio() {
  // Determine the delay for the current song based on its ID
  unsigned long songDelay = 0;
  switch (currentSong) {
    case 3:
      songDelay = 50;
      break;
    case 4:
      songDelay = 41;
      break;
    case 5:
      songDelay = 33;
      break;
    default:
      Serial.println("Error: Invalid current song ID");
      return; // Exit the function if currentSong is invalid
  }

  // Check if it's time to play the next song
  unsigned long currentTime = millis();
  if (currentTime - lastSongTick >= (songDelay * 1000) || lastSongTick == 0) {
    // Play the current song
    Serial.print("Now playing song ");
    Serial.println(currentSong);
    DFMini.play(currentSong);

    // Increment currentSong and wrap around if necessary
    currentSong++;
    if (currentSong > 5) {
      currentSong = 3; // Reset to the first valid song ID
    }

    // Update lastSongTick to the current time
    lastSongTick = currentTime;
  } else {
    Serial.println("Debouncing...");
  }
}

/**
 * Handles hit detection and associated events
 */
bool checkForHits() {
  bool hitDetected = false;
  if (goalHit) {
    hitDetected = true;
    // Play goal sound
    DFMini.play(6);
    
    // Update points, goals, and display
    points += 100;
    goals++;
    updateDisplay(points, goals);
    
    // Reset debounce flag
    goalHit = false;
  }
  if (goalieHit) {
    hitDetected = true;
    // Play goalie save sound
    DFMini.play(2);
    points -= 5;
    updateDisplay(points, goals);

    // Reset debounce flag
    goalieHit = false;
  }
  if (defenderHit) {
    hitDetected = true;
    // Play defender save sound
    DFMini.play(2);
    points -= 10;
    updateDisplay(points, goals);

    // Reset debounce flag
    defenderHit = false;
  }
  return hitDetected;
}

/**
 * Updates the LCD with the given points and goals
 */
void updateDisplay(int newPoints, int newGoals) {
  // Clear points and goals line
  lcd.setCursor(0, 1);
  lcd.print("                ");
  // Record change
  lcd.setCursor(1, 1);
  lcd.print(newPoints);
  lcd.setCursor(12, 1);
  lcd.print(newGoals);
}
/////////////////////////////////////////////////////////

// INTERRUPTS
/////////////////////////////////////////////////////////
/**
 * Interrupt for goal detection
 */
void goalReaction() {
  if (millis() - lastGoalTime > DEBOUNCE_DELAY) {
    int state = digitalRead(GOAL_INTERRUPT_PIN);
    if (state == HIGH) {
      Serial.println("Triggered!");
      goalHit = true;
    }
    lastGoalTime = millis();
  }
}

/**
 * Interrupt for goalie hit detection
 */
void goalieReaction() {
  if (millis() - lastGoalieTime > DEBOUNCE_DELAY) {
    int state = digitalRead(GOALIE_INTERRUPT_PIN);
    if (state == HIGH) {
      Serial.println("Triggered!");
      goalieHit = true;
    }
    lastGoalieTime = millis();
  }
}

/**
 * Interrupt for defender hit detection
 */
void defenderReaction() {
  if (millis() - lastDefenderTime > DEBOUNCE_DELAY) {
    int state = digitalRead(DEFENDER_INTERRUPT_PIN);
    if (state == HIGH) {
      Serial.println("Triggered!");
      defenderHit = true;
    }
    lastDefenderTime = millis();
  }
}
/////////////////////////////////////////////////////////
