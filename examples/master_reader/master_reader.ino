// Wire Master Reader
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI slave device
// Refer to the "Wire Slave Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.

#include <Wire.h>

int addr=0x11;

void setup() {
  Wire.begin();        // join I2C bus (address optional for master)
  Serial.begin(115200);  // start serial for output

}

void loop() {
  Wire.requestFrom(addr, 16);    // request 6 bytes from slave device #8

  Serial.print("<");
  while (Wire.available()) { // slave may send less than requested
    int c = Wire.read() << 8 | Wire.read(); // receive a byte as character
    Serial.print(c);         // print the character
    Serial.print("|");
  }
  Serial.println(">");


  delay(10);

  if (Serial.available()) {
    int dataa = Serial.read();
    if (dataa == '1') {
      Wire.beginTransmission(addr);
      Wire.write(1);
      Wire.endTransmission();
    }
    if (dataa == '0') {
      Wire.beginTransmission(addr);
      Wire.write(0);
      Wire.endTransmission();
    }
    if (dataa == 'a') {
      Wire.beginTransmission(addr);
      Wire.write(255);
      Wire.endTransmission();
    }
    if (dataa == '3') {
      Wire.beginTransmission(addr);
      Wire.write(0b111);
      Wire.endTransmission();
    }
  }

  delay(40);
}
