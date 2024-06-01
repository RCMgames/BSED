#include <Arduino.h>
const uint8_t PORTB_used_pins_mask = 0b00111111; // D13-D8
volatile uint8_t lastPortBPinStates = PORTB_used_pins_mask;
const uint8_t PORTC_used_pins_mask = 0b00001111; // A3-A0
volatile uint8_t lastPortCPinStates = PORTC_used_pins_mask;
const uint8_t PORTD_used_pins_mask = 0b11111100; // D7-D2
volatile uint8_t lastPortDPinStates = PORTD_used_pins_mask;

volatile uint16_t encoderCount[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

volatile uint8_t requestNumber = 255;

volatile uint8_t dataToSend[17] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

volatile int16_t onReceiveData = 0;

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

    twi_init(14 + digitalRead(0) + 2 * digitalRead(1)); // 14,15,16,17

    sei();
}

void loop()
{
}

// lots of I2C code from https://github.com/arduino/ArduinoCore-avr/blob/321fca0bac806bdd36af8afbc13587f4b67eb5f1/libraries/Wire/src/utility/twi.c (but modified)

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define TW_ST_SLA_ACK 0xA8
#define TW_ST_ARB_LOST_SLA_ACK 0xB0
#define TW_ST_DATA_ACK 0xB8
#define TW_ST_DATA_NACK 0xC0
#define TW_ST_LAST_DATA 0xC8
#define TW_SR_SLA_ACK 0x60
#define TW_SR_ARB_LOST_SLA_ACK 0x68
#define TW_SR_GCALL_ACK 0x70
#define TW_SR_ARB_LOST_GCALL_ACK 0x78
#define TW_SR_DATA_ACK 0x80
#define TW_SR_DATA_NACK 0x88
#define TW_SR_GCALL_DATA_ACK 0x90
#define TW_SR_GCALL_DATA_NACK 0x98
#define TW_SR_STOP 0xA0
#define TW_NO_INFO 0xF8
#define TW_BUS_ERROR 0x00
#define TW_STATUS_MASK (_BV(TWS7) | _BV(TWS6) | _BV(TWS5) | _BV(TWS4) | _BV(TWS3))
#define TW_STATUS (TWSR & TW_STATUS_MASK)
#define TW_READ 1
#define TW_WRITE 0

#define TWI_READY 0
#define TWI_SRX 3
#define TWI_STX 4
#define TWI_BUFFER_SIZE 17

volatile uint8_t twi_state;
static volatile uint8_t twi_txBufferIndex;
static volatile uint8_t twi_txBufferLength;
static uint8_t twi_rxBuffer[TWI_BUFFER_SIZE];
static volatile uint8_t twi_rxBufferIndex;
volatile uint8_t twi_error;

void twi_init(uint8_t address)
{
    twi_state = TWI_READY;
    // activate internal pullups for twi.
    digitalWrite(SDA, 1);
    digitalWrite(SCL, 1);

    TWAR = address << 1; // set twi peripheral address (skip over TWGCE bit)

    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
}
void twi_releaseBus(void)
{
    // release bus
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT);

    // update twi state
    twi_state = TWI_READY;
}

inline void twi_reply(uint8_t ack)
{
    // transmit controller read ready signal, with or without ack
    if (ack) {
        TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
    } else {
        TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
    }
}

/*
 * Function twi_stop
 * Desc     relinquishes bus controller status
 * Input    none
 * Output   none
 */
void twi_stop(void)
{
    // send stop condition
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTO);
    while (TWCR & _BV(TWSTO)) {
        continue;
    }
    // update twi state
    twi_state = TWI_READY;
}

inline void onReceive(uint8_t* dataBuf, int nData)
{
    onReceiveData = dataBuf[0];
    if (onReceiveData == 0) {
        encoderCount[0] = 0;
        encoderCount[1] = 0;
        encoderCount[2] = 0;
        encoderCount[3] = 0;
        encoderCount[4] = 0;
        encoderCount[5] = 0;
        encoderCount[6] = 0;
        encoderCount[7] = 0;
    } else if (onReceiveData > 0 && onReceiveData <= 0xFF) {
        requestNumber = onReceiveData;
        twi_txBufferLength = 0;
        twi_txBufferIndex = 0;
        if (bitRead(requestNumber, 7)) {
            dataToSend[twi_txBufferLength] = highByte(encoderCount[0]);
            twi_txBufferLength++;
            dataToSend[twi_txBufferLength] = lowByte(encoderCount[0]);
            twi_txBufferLength++;
        }
        if (bitRead(requestNumber, 6)) {
            dataToSend[twi_txBufferLength] = highByte(encoderCount[1]);
            twi_txBufferLength++;
            dataToSend[twi_txBufferLength] = lowByte(encoderCount[1]);
            twi_txBufferLength++;
        }
        if (bitRead(requestNumber, 5)) {
            dataToSend[twi_txBufferLength] = highByte(encoderCount[2]);
            twi_txBufferLength++;
            dataToSend[twi_txBufferLength] = lowByte(encoderCount[2]);
            twi_txBufferLength++;
        }
        if (bitRead(requestNumber, 4)) {
            dataToSend[twi_txBufferLength] = highByte(encoderCount[3]);
            twi_txBufferLength++;
            dataToSend[twi_txBufferLength] = lowByte(encoderCount[3]);
            twi_txBufferLength++;
        }
        if (bitRead(requestNumber, 3)) {
            dataToSend[twi_txBufferLength] = highByte(encoderCount[4]);
            twi_txBufferLength++;
            dataToSend[twi_txBufferLength] = lowByte(encoderCount[4]);
            twi_txBufferLength++;
        }
        if (bitRead(requestNumber, 2)) {
            dataToSend[twi_txBufferLength] = highByte(encoderCount[5]);
            twi_txBufferLength++;
            dataToSend[twi_txBufferLength] = lowByte(encoderCount[5]);
            twi_txBufferLength++;
        }
        if (bitRead(requestNumber, 1)) {
            dataToSend[twi_txBufferLength] = highByte(encoderCount[6]);
            twi_txBufferLength++;
            dataToSend[twi_txBufferLength] = lowByte(encoderCount[6]);
            twi_txBufferLength++;
        }
        if (bitRead(requestNumber, 0)) {
            dataToSend[twi_txBufferLength] = highByte(encoderCount[7]);
            twi_txBufferLength++;
            dataToSend[twi_txBufferLength] = lowByte(encoderCount[7]);
            twi_txBufferLength++;
        }
    }
}

ISR(TWI_vect)
{
    switch (TW_STATUS) {
        // peripheral Receiver
    case TW_SR_SLA_ACK: // addressed, returned ack
    case TW_SR_GCALL_ACK: // addressed generally, returned ack
    case TW_SR_ARB_LOST_SLA_ACK: // lost arbitration, returned ack
    case TW_SR_ARB_LOST_GCALL_ACK: // lost arbitration, returned ack
        // enter peripheral receiver mode
        twi_state = TWI_SRX;
        // indicate that rx buffer can be overwritten and ack
        twi_rxBufferIndex = 0;
        twi_reply(1);
        break;
    case TW_SR_DATA_ACK: // data received, returned ack
    case TW_SR_GCALL_DATA_ACK: // data received generally, returned ack
        // if there is still room in the rx buffer
        if (twi_rxBufferIndex < TWI_BUFFER_SIZE) {
            // put byte in buffer and ack
            twi_rxBuffer[twi_rxBufferIndex++] = TWDR;
            twi_reply(1);
        } else {
            // otherwise nack
            twi_reply(0);
        }
        break;
    case TW_SR_STOP: // stop or repeated start condition received
        // ack future responses and leave peripheral receiver state
        twi_releaseBus();
        // put a null char after data if there's room
        if (twi_rxBufferIndex < TWI_BUFFER_SIZE) {
            twi_rxBuffer[twi_rxBufferIndex] = '\0';
        }
        // callback to user defined callback
        onReceive(twi_rxBuffer, twi_rxBufferIndex);
        // since we submit rx buffer to "wire" library, we can reset it
        twi_rxBufferIndex = 0;
        break;
    case TW_SR_DATA_NACK: // data received, returned nack
    case TW_SR_GCALL_DATA_NACK: // data received generally, returned nack
        // nack back at controller
        twi_reply(0);
        break;

    // peripheral Transmitter
    case TW_ST_SLA_ACK: // addressed, returned ack
    case TW_ST_ARB_LOST_SLA_ACK: // arbitration lost, returned ack
        // enter peripheral transmitter mode
        twi_state = TWI_STX;
        // ready the tx buffer index for iteration
        // set tx buffer length to be zero, to verify if user changes it
        // request for txBuffer to be filled and length to be set
        // note: user must call twi_transmit(bytes, length) to do this
        // onRequest();
        // transmit first byte from buffer, fall
        /* fall through */
    case TW_ST_DATA_ACK: // byte sent, ack returned
        // copy data to output register
        TWDR = dataToSend[twi_txBufferIndex++];
        // if there is more to send, ack, otherwise nack
        if (twi_txBufferIndex >= 17) { // don't overflow dataToSend (there's an extra byte at the end)
            twi_txBufferIndex = 16;
        }
        twi_reply(0); // send bytes one at a time
        break;
    case TW_ST_DATA_NACK: // received nack, we are done
    case TW_ST_LAST_DATA: // received ack, but we are done already!
        // ack future responses
        twi_reply(1);
        // leave peripheral receiver state
        twi_state = TWI_READY;
        break;

    // All
    case TW_NO_INFO: // no state information
        break;
    case TW_BUS_ERROR: // bus error, illegal stop/start
        twi_error = TW_BUS_ERROR;
        twi_stop();
        break;
    }
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
