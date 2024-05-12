/**
 * Stepper/Servo handler for goalie system for ECE484 final project,
 * Cardboard Pinball.
 * 
 * Author: Chris Lallo, Blake Rile
 * Date: 5/6/2024
 */

#include <AccelStepper.h>
#include <Servo.h>

// CONSTANTS
const long GOALIE_DISTANCE = 8750;
const long GOALIE_BUFFER = 200;

AccelStepper myStepper = AccelStepper(8, 8, 9, 10, 11);
Servo goalie;

bool setZeroDebounce = false;

void setup() {
  // initialize the serial port:
  Serial.begin(9600);

  // Setup signal wire for raising servo
  pinMode(5, INPUT);

  // Setup stepper motor
  myStepper.setMaxSpeed(800);
  myStepper.setAcceleration(500);
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), setZero, FALLING);

  // Setup servo
  goalie.attach(3);
  goalie.write(90);
}

void loop() {
  while (digitalRead(5) == LOW) {
    // Wait until start...
  }
  while (digitalRead(2) == HIGH) {
    myStepper.move(-100);
    myStepper.run();
  }
  if (digitalRead(5) == HIGH) {
    flipServo(false);
  }
  myStepper.moveTo(GOALIE_DISTANCE);
  myStepper.runToPosition();
  myStepper.moveTo(GOALIE_BUFFER);
  setZeroDebounce = false;
  myStepper.runToPosition();
}

/**
 * Sets the zero position of the stepper motor when switch is activated (signal goes low)
 */
void setZero() {
  if (!setZeroDebounce) {
    setZeroDebounce = true;
    Serial.println("Goalie position set new zero!");
    myStepper.setCurrentPosition(0);
  }
  Serial.println("Debounced!");
}


/**
 * Flips the servo up or down when within the appropriate bounds
 */
void flipServo(bool up) {
  if (up) {
    goalie.write(90);
  } else {
    goalie.write(0);
  }
}
