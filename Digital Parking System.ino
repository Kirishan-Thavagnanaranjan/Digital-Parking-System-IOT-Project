#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo entranceServo, exitServo;

const int trigEntrance = 8, echoEntrance = 9;
const int trigExit = 10, echoExit = 11;

const int entranceRedLED = 2, entranceGreenLED = 3;
const int exitRedLED = 4, exitGreenLED = 5;

const int slotSensors[] = {A0, A1, A2, A3};
const int totalSlots = 4;

bool slotStatus[totalSlots] = {false, false, false, false};
int freeSlots = totalSlots;

const int detectionRange = 7;

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(entranceRedLED, OUTPUT);
  pinMode(entranceGreenLED, OUTPUT);
  pinMode(exitRedLED, OUTPUT);
  pinMode(exitGreenLED, OUTPUT);

  digitalWrite(entranceRedLED, HIGH);
  digitalWrite(entranceGreenLED, LOW);
  digitalWrite(exitRedLED, HIGH);
  digitalWrite(exitGreenLED, LOW);

  Serial.begin(9600);

  entranceServo.attach(6);
  exitServo.attach(7);
  entranceServo.write(0);
  exitServo.write(0);

  pinMode(trigEntrance, OUTPUT);
  pinMode(echoEntrance, INPUT);
  pinMode(trigExit, OUTPUT);
  pinMode(echoExit, INPUT);

  for (int i = 0; i < totalSlots; i++) {
    pinMode(slotSensors[i], INPUT);
  }

  displayWelcomeMessage();
}

void loop() {
  if (measureDistance(trigEntrance, echoEntrance) < detectionRange) {
    handleEntrance();
  }

  if (measureDistance(trigExit, echoExit) < detectionRange) {
    handleExit();
  }

  delay(500);
}

// Default Welcome Message
void displayWelcomeMessage() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("WELCOME!");
  lcd.setCursor(0, 1);
  lcd.print("Enjoy Your Visit");
  delay(2000);
}

// Measure distance using ultrasonic sensor
float measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  return (duration * 0.0343) / 2;
}

// Update slot status
void updateSlotStatus() {
  freeSlots = totalSlots;
  for (int i = 0; i < totalSlots; i++) {
    slotStatus[i] = !digitalRead(slotSensors[i]);
    if (slotStatus[i]) freeSlots--;
  }
}

// Get the first available slot
int getFirstAvailableSlot() {
  for (int i = 0; i < totalSlots; i++) {
    if (!slotStatus[i]) return i + 1;
  }
  return -1; // No slot available
}

// Handle Entrance
void handleEntrance() {
  updateSlotStatus();

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("WELCOME!");
  lcd.setCursor(0, 1);
  lcd.print("Please Wait");
  delay(2000);

  if (freeSlots > 0) {
    int assignedSlot = getFirstAvailableSlot();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Slot Available:");
    lcd.setCursor(0, 1);
    lcd.print("Go to Slot ");
    lcd.print(assignedSlot);
    delay(2500);

    blinkGreenLEDAndOpenGate(entranceServo, entranceRedLED, entranceGreenLED);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Parking Full!");
    lcd.setCursor(0, 1);
    lcd.print("Please Wait...");
    blinkFastRedLED(entranceRedLED);
    delay(2500);
  }

  displayWelcomeMessage(); // Return to welcome message after processing
}

// Handle Exit
void handleExit() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Thank You!");
  lcd.setCursor(0, 1);
  lcd.print("Visit Again");
  delay(2000);

  blinkGreenLEDAndOpenGate(exitServo, exitRedLED, exitGreenLED);
  updateSlotStatus();

  displayWelcomeMessage(); // Return to welcome message after exit
}

// Open gate & blink green LED
void blinkGreenLEDAndOpenGate(Servo &gateServo, int redLED, int greenLED) {
  digitalWrite(redLED, LOW);
  
  for (int i = 0; i < 3; i++) {
    digitalWrite(greenLED, HIGH);
    delay(500);
    digitalWrite(greenLED, LOW);
    delay(500);
  }

  digitalWrite(greenLED, HIGH);
  gateServo.write(90);
  delay(5000);
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, HIGH);
  gateServo.write(0);
}

// Blink red LED fast
void blinkFastRedLED(int redLED) {
  for (int i = 0; i < 5; i++) {
    digitalWrite(redLED, HIGH);
    delay(300);
    digitalWrite(redLED, LOW);
    delay(300);
  }
}
