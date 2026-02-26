// This is a sketch, designed to use an Arduino UNO, two servos, LCD screen and a LIDAR sensor to create a LIDAR scanner.
// The scanner with attempt to map out a room using this sketch, and a processing sktech.
// Run this first before the processing sketch.

// Importing libaries required for components to function
#include <LiquidCrystal.h>
#include <Servo.h>
#include <LIDARLite.h>

// Creating objects for both servos, the lidar, and the LCD screen.
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
// If LCD Keypad Shield is pre-soldered(Like in my case); pin assignment comes from the shield datasheet
Servo servoX;
Servo servoY;
LIDARLite lidar;

// Minimum/Maximum servo angles given in degrees.
// Please modify these to avoid hitting limits and allow for the best/desired range of motion for the scanner.
int minPosX = 69;
int maxPosX = 170;
int minPosY = 110;
int maxPosY = 180;

// Stores previous servo postions
// Used to detect movement
int lastPosX = 0;
int lastPosY = 0;

// The variables are used to read buttons from LCD Keypad
// The pin A0 connects all 5 buttons(Left, Up, Down, Right and Select) to one pin.
// Each button has a different voltage which will be used to read the button pressed.
int buttonPin = A0;
// Variable to store voltage detected from buttonPin
int buttonValue = 0;
// Variable creating a voltage threshold for button values. (Countering analog noise)
int buttonThreshold = 50;

//Counts how many times loop() has run
int loopCount = 0;

// Counts how many times auto-scan has run
int scanIteration = 0;

// Stores the current/previous LIDAR distance readings in centimeters
int radius = 0;
int lastRadius = 0;

// Tracks if SELECT button has already been handled
// Ensures the mode only changes once per physical press
boolean selectButtonPressed = false;

// Tracks wether auto-scan mode is active
boolean scanning = false;

// Direction of horizontal scan
// false = left
// true = right
boolean scanDirection = false;

// Stores how many degrees the servo moves each loop, in auto-scan mode.
// Smaller number = slower, more detailed scan
// Larger number = faster, less deteailed scan
// At very large numbers, can be jerky
int scanIncrement = 1;

// Starts servoX centered
int posX = (maxPosX + minPosX) / 2;

// Starts servoY at the lower fourth, (just low-ish)
int posY = (maxPosY + minPosY) / 4;

// Used to convert degrees into radians for trig functions
float pi = 3.14159265;
float deg2rad = pi / 180.0;


void setup() {
  // Initalises an LCD screen of 16x2 size
  // Change to your screen dimensions
  lcd.begin(16, 2);

  // Displays "Manual" or "Scanning" on the screen
  updateModeDisplay();

  // Initalises communication with Lidar scanner
  lidar.begin(0, true);

  // Configured to pick a mode, for lidar configuration, refer to library configuration for these values.
  // For the Lidar-Lite 3 Laser Rangefinder
  // 0: Default mode, balanced performance
  // 1: Short range, high speed. (Good for small rooms)
  // 2: Default range, higher speed short range. Turns on quick termination for faster measurements at short range, (with decreased accuracy)
  // 3: Maximum range, slow
  // 4: High sensitivity detection
  // 5: Low sensitivity detection
  lidar.configure(0);

  // Assigns servoX and servoY to the pins they are wired to.
  // In my case, D2 and D3
  servoX.attach(2);
  servoY.attach(3);

  // Move servos to starting position
  servoX.write(posX);
  servoY.write(posY);

  // Starts USB serial communication
  Serial.begin(9600);
}



void loop() {

  // Assigns voltage value, to buttonValue based on the button pressed.
  // Voltage values:
  // None ≈ 1023
  // Left ≈ 505
  // Up ≈ 145
  // Down ≈ 329
  // Right ≈ 0
  // Select ≈ 741
  buttonValue = analogRead(buttonPin);

  // If auto scan mode is active
  if (scanning == true) {

    // Detects if the SELECT button is pressed
    if (abs(buttonValue - 741) < buttonThreshold) {
      if (!selectButtonPressed) {
        // switch to manual scan mode
        selectButtonPressed = true;
        scanning = false;
        updateModeDisplay();
      }
    } else {
      selectButtonPressed = false;
    }

    // Moves servo to the left or right
    if (scanDirection == true) {
      posX += scanIncrement;
    } else {
      posX -= scanIncrement;
    }

    // If horziontal limit reached
    if (posX > maxPosX || posX < minPosX) {
      // Reverse scanning direction and move up one row
      scanDirection = !scanDirection;
      posY += scanIncrement;

      // End scan when top is reached
      if (posY > maxPosY) {
        if (scanIteration < 8) {
          posX = minPosX;
          posY = minPosY;
          scanDirection = true;
          scanIteration += 1;
        }
        else {
        // completed auto scan, return to manual scan mode
        scanning = false;
        updateModeDisplay();
        }
      }
    }

   // If auto scam mode is not active // Manual Mode 
  } else {

    // Detects if the SELECT button is pressed
    if (abs(buttonValue - 741) < buttonThreshold) {
      if (!selectButtonPressed) {
        // switch to auto scan mode
        selectButtonPressed = true;
        scanning = true;
        // Assign servos to default positions
        posX = minPosX;
        posY = minPosY;
        scanDirection = true;
        updateModeDisplay();
      }

    //Checks wether Left, Right, Top or Button button is pressed.  
    // Moves according to it
    } else if (abs(buttonValue - 505) < buttonThreshold) {
      // manual scan left
      posX -= 1;
    } else if (abs(buttonValue - 329) < buttonThreshold) {
      // manual scan down
      posY -= 1;
    } else if (abs(buttonValue - 145) < buttonThreshold) {
      // manual scan up
      posY += 1;
    } else if (abs(buttonValue - 0) < buttonThreshold) {
      // manual scan right
      posX += 1;
    } else {
      selectButtonPressed = false;
    }
  }

  // Ensures the values are within limits, preventing the servos from being damaged.
  posX = min(max(posX, minPosX), maxPosX);
  posY = min(max(posY, minPosY), maxPosY);
  
  // Moves servos only if position changed.
  bool moved = moveServos();

  // Updates LCD with current angles
  displayPosition();

  // Increment loop count
  loopCount += 1;

  // Recalibrate scanner every 100 scans
  if (loopCount % 100 == 0) {
    radius = lidar.distance();

    // Faster read without any calibration
  } else {
    radius = lidar.distance(false);
  }

  // Only updates radius, if distance detected moves more then 2cm, (filters out analogue noise)
  if (abs(radius - lastRadius) > 2)
  {
    lastRadius = radius;
    // Sets LCD cursor to 8th column 0 row
    lcd.setCursor(8, 0);

    // Displays distance in meters.
    lcd.print("D:" + String(radius / 100.0, 2) + "  ");
  }
  if (scanning == true || moved == true) {

    // Converts servo angles to spherical coordinates
    float azimuth = posX * deg2rad;
    float elevation = (180 - maxPosY + posY) * deg2rad;

    // Converts spherical to Cartesian coordinates
    double x = radius * sin(elevation) * cos(azimuth);
    double y = radius * sin(elevation) * sin(azimuth);
    double z = radius * cos(elevation);

    // Send 3D point to serial monitor, to be used in processing sketch.
    Serial.println(String(-x, 5) + " " + String(y, 5) + " " + String(-z, 5));
  }
}

// Function for moving servos
bool moveServos()
{
  
  // Keeps track wether either servo actually moved this loop
  bool moved = false;
  // Remebers the last servo posiitons between different function call iterations
  static int lastPosX;
  static int lastPosY;

  // If servo position has changed, move the servos!
  if (posX != lastPosX) {
    servoX.write(posX);
    lastPosX = posX;
    moved = true;
  }
  if (posY != lastPosY) {
    servoY.write(posY);
    lastPosY = posY;
    moved = true;
  }

  // Waits 30ms before next iteration
  delay(60);

  // Tells program if servo moved
  return moved;
}

void displayPosition()
{

  // Remebers previous X/Y positions displayed on LCD
  static int lastPosX;
  static int lastPosY;

  // If servo position has changed, update the screen!
  if (posX != lastPosX) {
    lcd.setCursor(0, 0);
    lcd.print("X:" + String(posX) + "  ");
    lastPosX = posX;
  }
  if (posY != lastPosY) {
    lcd.setCursor(0, 1);
    lcd.print("Y:" + String(posY) + "  ");
    lastPosY = posY;
  }  
}

// Changes the display depending if auto-scan mode is active or inactive
void updateModeDisplay()
{
  lcd.setCursor(8, 1);
  if (scanning) {
    lcd.print("Scanning");
  } else {
    lcd.print("Manual  ");
  }
}


