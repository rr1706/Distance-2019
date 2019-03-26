/**
 * Handles multiple VL53L0X sensors and provides digital signals to 
 * the RoboRio when certain sensor patterns are found.
 * 
 * Requires: https://github.com/rr1706/vl53l0x-arduino
 *           https://github.com/rr1706/vl53l0x-rr1706
 *           https://github.com/Testato/SoftwareWire
 */

#include <SoftwareWire.h>
#include <Wire.h>
#include <FastLED.h>
#include <SoftVL53L1X.h>

// For LED feedback
#define NUM_LEDS 20
#define LED_DATA_PIN 12
#define LED_CLOCK_PIN 13

#define COMMON_I2C_CLOCK_PIN 2
#define NUM_SENSORS 7

#define IS_I2C_SLAVE true
#define USE_DIAG_LIGHTS false
#define USE_OLED_DISPLAY false

#define GOOD_SENSOR_DELAY 5

template< typename T, size_t N > size_t ArraySize (T (&) [N]){ return N; }

const String goodValues[] = {"12","2","3","23","13","24","34","124","134"};
const String actionableValues[] = {"1","4","14","123","234","1234"};
const String wallValues[] = {"123","234","1234"};

CRGB leds[USE_DIAG_LIGHTS ? NUM_LEDS : 1] = {CRGB::Blue};

// Create an array of Software I2C interfaces, one for each sensor.
SoftwareWire* wires[NUM_SENSORS];
SoftVL53L1X* sensors[NUM_SENSORS];
SoftwareWire *oledDisplayWire;

unsigned short distances[NUM_SENSORS] = {0};
byte  readIndex = 0;
int   currentGoodReadingCount = GOOD_SENSOR_DELAY;
bool  debugging = false;
int   counter = 0;
int   heartbeat = 0;
bool  sensorsExist[NUM_SENSORS];
int readCount = 0;
unsigned long time_now = 0;

void setup() {

  // Wait for serial port to come online.
  while(!Serial);
  Serial.begin(115200);

  //Serial.print("Number of good items: ");Serial.println(ArraySize(goodValues));

  if (IS_I2C_SLAVE) {
    // Start the i2c interface as slave at address 8.
    Wire.begin(8);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
    Serial.println("Listening on I2C");
  }
  
  // Initialize the range finders...
  for (int i = 0; i < NUM_SENSORS; i++) {
    wires[i] = new SoftwareWire(i + 3, COMMON_I2C_CLOCK_PIN);
    //wires[i]->setClock(400000UL);
    wires[i]->begin();
    sensors[i] = new SoftVL53L1X(wires[i]);

    if (!sensors[i]->init()) {
      sensorsExist[i] = false;
      Serial.print("Sensor not available in socket #"); Serial.println(i);
    } else {
      sensorsExist[i] = true;
      sensors[i]->setTimeout(500);
      sensors[i]->setDistanceMode(VL53L1X::Medium);
      sensors[i]->setMeasurementTimingBudget(40000);
      sensors[i]->startContinuous(40);
    }
  }

  // Set up analog pins as outputs to send signals to RoboRIO.
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);

  if (!IS_I2C_SLAVE) {
    pinMode(A4, OUTPUT);
    pinMode(A5, OUTPUT);
  }

  if (USE_DIAG_LIGHTS) {
    // LEDs
    FastLED.addLeds<APA102, LED_DATA_PIN, LED_CLOCK_PIN, BGR, DATA_RATE_MHZ(12)>(leds, NUM_LEDS);
    // The Brightness maxium is 255
    FastLED.setBrightness(10);
    for (int i = 0; i < NUM_LEDS; i++) {
     // leds[i] = CRGB::Blue;
    }
  } else if (USE_OLED_DISPLAY) {
    oledDisplayWire = new SoftwareWire(10, COMMON_I2C_CLOCK_PIN, true, false);
    oledDisplayWire->setTimeout(25); // 25ms
    oledDisplayWire->setClock(400000UL);
    oledDisplayWire->begin();
    initOledDisplay(*oledDisplayWire);
  }

  Serial.println("Ready for action.  Enter 1 to enable debugging, or 0 to disable.");

}


/**
 * Hardware I2C slave function to send data to the I2C master on request.
 */
void requestEvent() {
  readCount++;
  byte dataToSend[NUM_SENSORS * 2] = {0};
  for (int i = 0; i < NUM_SENSORS; i++) {
    short data = distances[i]; // * 10 / 254;
    dataToSend[i * 2] = (data >> 8) & 0xff;
    dataToSend[i * 2 + 1] = data & 0xff;
  }
  Wire.write(dataToSend, NUM_SENSORS * 2);
}

/**
 * Hardware I2C slave function to receive data from the I2C master.
 */
void receiveEvent(int howMany) {
  readCount++;
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
  }
  readIndex = Wire.read();    // receive byte as an integer
}


/**
 * Main loop.
 */
void loop() {

  time_now = millis();

  // read inputs
  for (int i = 0; i < NUM_SENSORS; i++) {
    short lastReading = distances[i];
    //if (debugging) { Serial.print("Reading sensor #"); Serial.println(i); }
    if (sensorsExist[i]) {
      distances[i] = sensors[i]->read(); //processSensor(wires[i], distances[i]);
      if (sensors[i]->timeoutOccurred()) {
        if (debugging) { Serial.print("Timeour detected, re-initializing sensor #"); Serial.print(i); }
        if (sensors[i]->init()) {
          sensors[i]->setTimeout(500);
          sensors[i]->setDistanceMode(VL53L1X::Medium);
          sensors[i]->setMeasurementTimingBudget(50000);
          sensors[i]->startContinuous(50);
        }
        distances[i] = lastReading;
        if (debugging) { Serial.println("..."); }
      }
    }
  }


// Debugging output
  if (debugging) {
  
    for (int i = 0; i < NUM_SENSORS; i++) {
      Serial.print(distances[i]); // * 10 / 254);
      Serial.print("\t");
      //Serial.print(distances[i] / 25.4);
    }
  
    Serial.println();
  }

  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '0') {
      debugging = false;
    } else if (c == '1') {
      debugging = true;
    }
  }

//  if (USE_DIAG_LIGHTS) {
//    leds[0] = distances[0] > MIN_CUBE_DISTANCE && distances[0] < NEAR_CUBE_DISTANCE ? CRGB::Blue : CRGB::Red;
//    leds[1] = distances[1] > MIN_CUBE_DISTANCE && distances[1] < NEAR_CUBE_DISTANCE ? CRGB::Blue : CRGB::Red;
//    leds[2] = distances[2] > MIN_CUBE_DISTANCE && distances[2] < NEAR_CUBE_DISTANCE ? CRGB::Blue : CRGB::Red;
//    leds[3] = distances[3] > MIN_CUBE_DISTANCE && distances[3] < NEAR_CUBE_DISTANCE ? CRGB::Blue : CRGB::Red;
//  
//    leds[5] = cubeInPosition ? CRGB::Blue : CRGB::Red;
//    leds[6] = cubeActionable ? CRGB::Blue : CRGB::Red;
//  
//    leds[8] = hasCubeLow ? CRGB::Blue : CRGB::Red;
//    leds[9] = hasCubeHigh ? CRGB::Blue : CRGB::Red;
//  
//    leds[11] = foundStack ? CRGB::Blue : CRGB::Red;
//    leds[12] = againstWall ? CRGB::Blue : CRGB::Red;
//
//    int strandColor = CRGB::Blue;
//    int topMark = 14;
//    if (hasCubeHigh) {
//      topMark = NUM_LEDS;
//    } else if (hasCubeLow) {
//      topMark = 2 * NUM_LEDS / 3;
//    } else if (cubeInPosition) {
//      topMark = NUM_LEDS / 2;
//    } else if (cubeActionable) {
//      topMark = NUM_LEDS / 4;
//    } else {
//      topMark = NUM_LEDS;
//      strandColor = CRGB::Green;
//    }
//    for (int i = 14; i < NUM_LEDS; i++) {
//      leds[i] = (i < topMark) ? strandColor : CRGB::Black;
//    }
//    FastLED.show();
//  }

  if (USE_OLED_DISPLAY) {
    for (int q = 0; q < NUM_SENSORS; q++) {
      drawLine(*oledDisplayWire, q, distances[q] / 32);
    }
    drawLine(*oledDisplayWire, NUM_SENSORS, counter % 120);
  }
  
  if (!IS_I2C_SLAVE) {
    counter++;
    if (counter % 10 == 0) {
      heartbeat = 1 - heartbeat;
      digitalWrite(A5, heartbeat);
    }
  }

  while(millis() < time_now + 50) { 
    // Loop every 50 milliseconds.
  }

}
