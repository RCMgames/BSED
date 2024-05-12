#include <Arduino.h>
#include <Wire.h>
const uint8_t PORTB_used_pins_mask = 0b00111111; // D13-D8
volatile uint8_t lastPortBPinStates = PORTB_used_pins_mask;
const uint8_t PORTC_used_pins_mask = 0b00001111; // A3-A0
volatile uint8_t lastPortCPinStates = PORTC_used_pins_mask;
const uint8_t PORTD_used_pins_mask = 0b11111100; // D7-D2
volatile uint8_t lastPortDPinStates = PORTD_used_pins_mask;

volatile uint16_t encoderCount[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

volatile uint8_t requestNumber = 255;

volatile uint8_t sendI = 0;

void onReceive(int numBytes)
{
    while (Wire.available()) {
        int16_t data = Wire.read();
        if (data == 0) {
            encoderCount[0] = 0;
            encoderCount[1] = 0;
            encoderCount[2] = 0;
            encoderCount[3] = 0;
            encoderCount[4] = 0;
            encoderCount[5] = 0;
            encoderCount[6] = 0;
            encoderCount[7] = 0;
        } else if (data > 0 && data <= 0xFF) {
            requestNumber = data;
            sendI = 0;
        }
    }
}
void onRequest()
{
    if (sendI == 0) {
        sendI++;
        if (bitRead(requestNumber, 7)) {
            Wire.write((uint8_t)(encoderCount[0] >> 8));
            Wire.write((uint8_t)(encoderCount[0] & 0xFF));
            return;
        }
    }
    if (sendI == 1) {
        sendI++;
        if (bitRead(requestNumber, 6)) {
            Wire.write((uint8_t)(encoderCount[1] >> 8));
            Wire.write((uint8_t)(encoderCount[1] & 0xFF));
            return;
        }
    }
    if (sendI == 2) {
        sendI++;
        if (bitRead(requestNumber, 5)) {
            Wire.write((uint8_t)(encoderCount[2] >> 8));
            Wire.write((uint8_t)(encoderCount[2] & 0xFF));
            return;
        }
    }
    if (sendI == 3) {
        sendI++;
        if (bitRead(requestNumber, 4)) {
            Wire.write((uint8_t)(encoderCount[3] >> 8));
            Wire.write((uint8_t)(encoderCount[3] & 0xFF));
            return;
        }
    }
    if (sendI == 4) {
        sendI++;
        if (bitRead(requestNumber, 3)) {
            Wire.write((uint8_t)(encoderCount[4] >> 8));
            Wire.write((uint8_t)(encoderCount[4] & 0xFF));
            return;
        }
    }
    if (sendI == 5) {
        sendI++;
        if (bitRead(requestNumber, 2)) {
            Wire.write((uint8_t)(encoderCount[5] >> 8));
            Wire.write((uint8_t)(encoderCount[5] & 0xFF));
            return;
        }
    }
    if (sendI == 6) {
        sendI++;
        if (bitRead(requestNumber, 1)) {
            Wire.write((uint8_t)(encoderCount[6] >> 8));
            Wire.write((uint8_t)(encoderCount[6] & 0xFF));
            return;
        }
    }
    if (sendI == 7) {
        sendI++;
        if (bitRead(requestNumber, 0)) {
            Wire.write((uint8_t)(encoderCount[7] >> 8));
            Wire.write((uint8_t)(encoderCount[7] & 0xFF));
            return;
        }
    }
    sendI = 0;
    Wire.write(0); // shouldn't get here
    Wire.write(0);
}

void setup()
{
    pinMode(0, INPUT_PULLUP); // for address
    pinMode(1, INPUT_PULLUP); // for address

    pinMode(2, INPUT_PULLUP); // encoder pins are INPUT_PULLUP to prevent unused pins from floating
    pinMode(3, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);
    pinMode(6, INPUT_PULLUP);
    pinMode(7, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);
    pinMode(9, INPUT_PULLUP);
    pinMode(10, INPUT_PULLUP);
    pinMode(11, INPUT_PULLUP);
    pinMode(12, INPUT_PULLUP);
    pinMode(13, INPUT_PULLUP);
    pinMode(A0, INPUT_PULLUP);
    pinMode(A1, INPUT_PULLUP);
    pinMode(A2, INPUT_PULLUP);
    pinMode(A3, INPUT_PULLUP);
    delay(1);

    cli();
    PCICR |= 0b00000111; // turn on all pin change interrupts
    PCMSK0 = PORTB_used_pins_mask;
    PCMSK1 = PORTC_used_pins_mask;
    PCMSK2 = PORTD_used_pins_mask;

    Wire.begin(14 + digitalRead(0) + 2 * digitalRead(1)); // 14,15,16,17

    Wire.onReceive(onReceive);
    Wire.onRequest(onRequest);

    sei();
}

void loop()
{
}

ISR(PCINT0_vect)
{
    // D13-D8
    uint8_t pinStates = PINB & PORTB_used_pins_mask;
    uint8_t changedPins = pinStates ^ lastPortBPinStates;
    lastPortBPinStates = pinStates;

    if (bitRead(changedPins, 0))
        (bitRead(pinStates, 0) == bitRead(pinStates, 1)) ? encoderCount[4]++ : encoderCount[4]--;
    if (bitRead(changedPins, 1))
        (bitRead(pinStates, 0) != bitRead(pinStates, 1)) ? encoderCount[4]++ : encoderCount[4]--;
    if (bitRead(changedPins, 2))
        (bitRead(pinStates, 2) == bitRead(pinStates, 3)) ? encoderCount[3]++ : encoderCount[3]--;
    if (bitRead(changedPins, 3))
        (bitRead(pinStates, 2) != bitRead(pinStates, 3)) ? encoderCount[3]++ : encoderCount[3]--;
    if (bitRead(changedPins, 4))
        (bitRead(pinStates, 4) == bitRead(pinStates, 5)) ? encoderCount[2]++ : encoderCount[2]--;
    if (bitRead(changedPins, 5))
        (bitRead(pinStates, 4) != bitRead(pinStates, 5)) ? encoderCount[2]++ : encoderCount[2]--;
}

ISR(PCINT1_vect)
{
    // A3-A0
    uint8_t pinStates = PINC & PORTC_used_pins_mask; // A3-A0
    uint8_t changedPins = pinStates ^ lastPortCPinStates;
    lastPortCPinStates = pinStates;
    if (bitRead(changedPins, 0))
        (bitRead(pinStates, 0) == bitRead(pinStates, 1)) ? encoderCount[1]++ : encoderCount[1]--;
    if (bitRead(changedPins, 1))
        (bitRead(pinStates, 0) != bitRead(pinStates, 1)) ? encoderCount[1]++ : encoderCount[1]--;
    if (bitRead(changedPins, 2))
        (bitRead(pinStates, 2) == bitRead(pinStates, 3)) ? encoderCount[0]-- : encoderCount[0]++;
    if (bitRead(changedPins, 3))
        (bitRead(pinStates, 2) != bitRead(pinStates, 3)) ? encoderCount[0]-- : encoderCount[0]++;
}
ISR(PCINT2_vect)
{
    uint8_t pinStates = PIND & PORTD_used_pins_mask; // D7-D2
    uint8_t changedPins = pinStates ^ lastPortDPinStates;
    lastPortDPinStates = pinStates;
    if (bitRead(changedPins, 2))
        (bitRead(pinStates, 2) == bitRead(pinStates, 3)) ? encoderCount[7]++ : encoderCount[7]--;
    if (bitRead(changedPins, 3))
        (bitRead(pinStates, 2) != bitRead(pinStates, 3)) ? encoderCount[7]++ : encoderCount[7]--;
    if (bitRead(changedPins, 4))
        (bitRead(pinStates, 4) == bitRead(pinStates, 5)) ? encoderCount[6]++ : encoderCount[6]--;
    if (bitRead(changedPins, 5))
        (bitRead(pinStates, 4) != bitRead(pinStates, 5)) ? encoderCount[6]++ : encoderCount[6]--;
    if (bitRead(changedPins, 6))
        (bitRead(pinStates, 6) == bitRead(pinStates, 7)) ? encoderCount[5]++ : encoderCount[5]--;
    if (bitRead(changedPins, 7))
        (bitRead(pinStates, 6) != bitRead(pinStates, 7)) ? encoderCount[5]++ : encoderCount[5]--;
}
