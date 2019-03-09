#include <SoftwareWire.h>

#define OLED_ADDRESS ((uint8_t)0x3C)
#define CONT_CMD_OP ((byte)0x80)
#define SNGL_CMD_OP ((byte)0x00)
#define WRITE_RAM_OP ((byte)0x40)


//int values[8];
int lastValues[8] = {0};

int writeCmd(SoftwareWire &myWire, byte cmd) {
  myWire.beginTransmission(OLED_ADDRESS);
  myWire.write(SNGL_CMD_OP);
  myWire.write(cmd);
  int code = 0;
  if ((code = myWire.endTransmission(true)) != 0) {
    Serial.print("Error writing command: "); Serial.println(code);
    return code;
  }
  return 0;
}

int writeDoubleCmd(SoftwareWire &myWire, byte cmd1, byte cmd2) {
  byte buffer[3];
  buffer[0] = SNGL_CMD_OP;
  buffer[1] = cmd1;
  buffer[2] = cmd2;
  myWire.beginTransmission(OLED_ADDRESS);
  myWire.write(buffer, 3);
  int code = 0;
  if ((code = myWire.endTransmission(true)) != 0) {
    //Serial.print("Error writing command: "); Serial.println(code);
    return code;
  }
  return 0;
}

void writeData(SoftwareWire &myWire, byte data) {
  byte buffer[2];
  buffer[0] = WRITE_RAM_OP;
  buffer[1] = data;
  myWire.beginTransmission(OLED_ADDRESS);
  myWire.write(buffer, 2);
  if (myWire.endTransmission(true) != 0) {
    //myWire.printStatus(Serial);
  }
}

int writeData(SoftwareWire &myWire, byte* data, int count) {
  myWire.beginTransmission(OLED_ADDRESS);
  myWire.write(WRITE_RAM_OP);
  myWire.write(data, count);
  int code = 0;
  if ((code = myWire.endTransmission(true)) != 0) {
    //Serial.print("Error writing Data: "); Serial.println(code);
    return code;
  }
  return 0;
}

void drawLine(SoftwareWire &myWire, int page, int value) {
  byte buffer[132] = {0};
  int startPoint = min(value, lastValues[page]);
  int endPoint = min(132, max(value + 1, lastValues[page]));
  for (int i = 0; i < value; i++) {
    buffer[i] = 0x03C;
  }
  writeCmd(myWire, (byte)0xB0 + page);  // Set page
  writeCmd(myWire, (byte)(startPoint & 0x0f));  // Set Column 0
  writeCmd(myWire, (byte)(16 + startPoint / 16));  // Set Column 0
  if (writeData(myWire, buffer + startPoint, endPoint - startPoint) != 0) {
    Serial.println("Re-initializing display");
    initOledDisplay(myWire);
  }
  lastValues[page] = value;
}

void initOledDisplay(SoftwareWire &myWire) {
  //delay(500);   // Give it a second to rest.
  writeCmd(myWire, (byte)0xAE);        // Display off
  writeDoubleCmd(myWire, 0xD5, 0x50);  // Clock divide ratio
  writeDoubleCmd(myWire, 0xA8, 0x3F);  // Multiplex ratio
  writeCmd(myWire, (byte)0x40);  // Set Start line to 0
  writeDoubleCmd(myWire, 0x81, 0xCF);  // Contrast control
  writeCmd(myWire, 0x32);              // Set pump voltage
  writeDoubleCmd(myWire, 0xD9, 0x22);  // Pre-cahrge period
  writeDoubleCmd(myWire, 0xAD, 0x8B);
  
  writeCmd(myWire, (byte)0xB0);  // Set page 0
  writeCmd(myWire, (byte)0x00);  // Set Column 0
  writeCmd(myWire, (byte)0x10);  // Set Column 0
  writeCmd(myWire, (byte)0xA6);  // Set Normal Display
  writeCmd(myWire, (byte)0xAF);  // Display on

}
