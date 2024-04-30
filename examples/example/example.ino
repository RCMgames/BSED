#include <Arduino.h>
#include <Wire.h>
#include <byte-sized-encoder-decoder.h>
/*
https://github.com/RCMgames/BSCD
*/
ByteSizedEncoderDecoder bsed = ByteSizedEncoderDecoder(&Wire, 0x11);
void setup()
{
    Serial.begin(115200);
    Wire.begin();
    bsed.begin();
}
void loop()
{
    bsed.run();
    for (int i = 0; i < 8; i++) {
        if (bsed.isEncoderActive(i)) {
            Serial.print(i);
            Serial.print(": ");
            Serial.print(bsed.getEncoderPosition(i));
            Serial.print(", ");
        }
    }
    Serial.println();
    delay(100);
}