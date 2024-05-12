#include <Arduino.h>
#include <Wire.h>
#include <byte-sized-encoder-decoder.h>
/*
https://github.com/RCMgames/BSED
*/
ByteSizedEncoderDecoder bsed = ByteSizedEncoderDecoder(&Wire1, 14);
void setup()
{
    Serial.begin(115200);
    Wire1.begin(SDA1, SCL1, 400000);
    bsed.begin();
    // bsed.setWhichEncoders(0b10010001);
}
void loop()
{
    bsed.run();
    for (int i = 1; i <= 8; i++) {
        if (bsed.isEncoderActive(i)) {
            Serial.print(i);
            Serial.print(": ");
            Serial.print(bsed.getEncoderPositionWithOverflows(i));
            Serial.print(", ");
        }
    }
    Serial.println();
    delay(10);
}