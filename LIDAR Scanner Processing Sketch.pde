// This is a sketch, designed to use an Arduino UNO, two servos, LCD screen and a LIDAR sensor to create a LIDAR scanner.
// The scanner with attempt to map out a room using an arduino sketch, and this sktech.
// Run this sketch after running the Arduino sketch

// This sketch accepts XYZ coordinates from Arduino LIDAR scanner
// and displays them graphically as a 3D point cloud that you can
// pan, zoom, and rotate using keyboard.

// Lets Processing talk to your arduino through USB (serial communication)
import processing.serial.*;

Serial serial;

//If you have multiple COM ports, this chooses which one to use. (If you have only plugged in one it will usually be 0)
int serialPortNumber = 0;
// Default rotation of the 3D view
float angle = 6.5f;
// How fast it rotates each frame
float angleIncrement = 0;
// Current position of the point cloud left/right
float xOffset = 3.0;
// Velocity of the movement of the point cloud left/right
float xOffsetIncrement = 0;
// Current position of the point cloud up/down
float yOffset = 152.0f;
// Velocity of the movement of the point cloud up/down
float yOffsetIncrement = 0;
// Current zoom level
float scale = 2.6f;
// Velocity of changing the zoom level
float scaleIncrement = 0;
// Stores all your Lidar 3D points
ArrayList<PVector> vectors;
// Used to flash point red for a few frames when generating new points
int lastPointIndex = 0;
int lastPointCount = 0;

// Runs once when the program starts
void setup() {
  // Creates a window with 3D rendering
  size(800, 600, P3D);
  // USES RGB colour system
  colorMode(RGB, 255, 255, 255);
  // Turns off anti-aliasing (makes points sharper).
  noSmooth();
  // Creates empty list to store points
  vectors = new ArrayList<PVector>();
  // Gets all avaliable COM ports
  String[] serialPorts = Serial.list();
  // Selects the port you chose above
  String serialPort = serialPorts[serialPortNumber];
  // Prints port used in console
  println("Using serial port \"" + serialPort + "\"");
  println("To use a different serial port, change serialPortNumber:");
  printArray(serialPorts);
  // Opens USB connection at 9600 baud. ( This must match Arduino baud rate ) 
  serial = new Serial(this, serialPort, 9600);
}

void draw() {
  // Reads one line from serial (Arduino IDE)
  String input = serial.readStringUntil(10);
  // Only continues if data is present
  if (input != null) {
    // Splits output into 3 pieces (x, y, z)
    String[] components = split(input, ' ');
    // If we got 3 numbers to plot.
    if (components.length == 3) {
      // Converts to float and stores as 3D point
      vectors.add(new PVector(float(components[0]), float(components[1]), float(components[2])));
    }
  }
  // Clear screen to black
  background(0);
  // Makes origin to centre of screen
  translate(width/2, height/2, -50);
  // Rotates entire 3D world left/right
  rotateY(angle);
  // Stores how many points we have
  int size = vectors.size();
  // Loops through every point
  for (int index = 0; index < size; index++) {
    // Gets one point
    PVector v = vectors.get(index);
    // If this is the newest point
    if (index == size - 1) {
      // Make newest point flash red briefly
      if (index == lastPointIndex) {
        // Increases counter if its the same point as before
        lastPointCount++;
      } else {
        // A new point arrived, then update newest index and reset counter
        lastPointIndex = index;
        lastPointCount = 0;
      }
      // If it hasn't drawn red for past 10 counts
      if (lastPointCount < 10) {
        // Draws red line from origin to new point
        stroke(255, 0, 0);
        line(xOffset, yOffset, 0, v.x * scale + xOffset, -v.z * scale + yOffset, -v.y * scale);
      }
    }
    // Draws the actual 3d point
    stroke(255, 255, 255);
    point(v.x * scale + xOffset, -v.z * scale + yOffset, -v.y * scale);
  }
  // Makes rotation, movement and zoom continous when key is held
  angle += angleIncrement;
  xOffset += xOffsetIncrement;
  yOffset += yOffsetIncrement;
  scale += scaleIncrement;
}

// Keyboard controls

// If key is pressed, rotation movement and zoom are activated!
void keyPressed() {
  if (key == 'q') {
    // zoom in
    scaleIncrement = 0.02f;
  } else if (key == 'z') {
    // zoom out
    scaleIncrement = -0.02f;
  } else if (key == 'a') {
    // move left
    xOffsetIncrement = -1f;
  } else if (key == 'd') {
    // move right
    xOffsetIncrement = 1f;
  } else if (key == 'w') {
    // move up
    yOffsetIncrement = -1f;
  } else if (key == 's') {
    // move down
    yOffsetIncrement = 1f;
  } else if (key =='x') {
    // erase all points
    vectors.clear();
  } else if (key == CODED) {
    if (keyCode == LEFT) {
      // rotate left
      angleIncrement = -0.015f;
    } else if (keyCode == RIGHT) {
      // rotate right
      angleIncrement = 0.015f;
    }
  }
}

// If key is released, rotation movement and zoom are stop!
void keyReleased() {
  if (key == 'q') {
    scaleIncrement = 0f;
  } else if (key == 'z') {
    scaleIncrement = 0f;
  } else if (key == 'a') {
    xOffsetIncrement = 0f;
  } else if (key == 'd') {
    xOffsetIncrement = 0f;
  } else if (key == 'w') {
    yOffsetIncrement = 0f;
  } else if (key == 's') {
    yOffsetIncrement = 0f;
  } else if (key == CODED) {
    if (keyCode == LEFT) {
      angleIncrement = 0f;
    } else if (keyCode == RIGHT) {
      angleIncrement = 0f;
    }
  }
}
