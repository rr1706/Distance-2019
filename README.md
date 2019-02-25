# Distance 2019 (FRC Deep Space)
Distance Sensor Controller - Arduino

This is a basic arduino project that uses Software i2c to talk to up to 8 VL53L0X or VL53L1X laser Time of Flight 
(rangfinder) sensors.  The sensor readings are used to determine if a game piece in position and ready 
for pickup.  They can also be used to determine if the robot is aligned to a wall or part of the field.

4 Sensors (#'s 0 - 3) are used across the front of the robot to detect the prsence of a game piece.

A fifth sensor is used in the gripper to determine if we have a good hold on an object.

Analog outputs are used as digital outs to send signals to the RoboRIO robot controller.
  * A0 -->  Cube fully engaged in intake gripper  (Active Low)
  * A1 -->  Cube in good position for pickup (Active High)
  * A2 -->  Robot against wall or field element (Active High)
  * A3 -->  Cube partially engaged in intake gripper (Active High)
  * A4 -->  TBD
  * A5 -->  Heartbeat - used as a visual diagnostic.
    
Outputs A4 and A5 can alternatively be used as an I2C slave so that the RoboRio can configure the sensor controller.

A strip of APA102 programmable color LEDs can be connected to pins 12 (data) and 13 (clock) to send diagnostic 
information about the state of the system to the LEDs.

This project uses several modofied libraries:
  * https://github.com/rr1706/vl53l0x-arduino
  * https://github.com/rr1706/SoftwareWire
  * https://github.com/rr1706/vl53l0x-rr1706
  * https://github.com/mhenman/vl53l1x-arduino


