// Define the pin numbers
const int buttonPin = 2;    // the number of the pushbutton pin
const int solenoidPin = 3;  // the number of the solenoid pin

// Variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

void setup() {
  // Initialize the solenoid pin as an output:
  pinMode(solenoidPin, OUTPUT);
  // Initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
}

void loop() {
  // Read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // Check if the pushbutton is pressed.
  // If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // Turn on the solenoid:
    digitalWrite(solenoidPin, HIGH);
    delay(1000); // Adjust delay as needed to control solenoid activation time
    // Turn off the solenoid:
    digitalWrite(solenoidPin, LOW);
  }
}
