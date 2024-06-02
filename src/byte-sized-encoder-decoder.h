#ifndef BYTE_SIZED_ENCODER_DECODER_H
#define BYTE_SIZED_ENCODER_DECODER_H
#include <Arduino.h>
#include <Wire.h>
/**
 * @brief class for communicating with the firmware on a Byte Sized Encoder Decoder board
 */
class ByteSizedEncoderDecoder {
protected:
    /**
     * @brief  I2C address of the Byte Sized Encoder Decoder board (as selected by the jumpers)
     */
    uint8_t address;
    /**
     * @brief  array of 8 numbers representing the number of steps each encoder has taken
     */
    int16_t encoderCount[8];
    /**
     * @brief  array of 8 numbers representing the number of times each encoder's counter variable has overflowed
     */
    int16_t encoderOverflows[8];
    /**
     * @brief  array of 8 numbers representing the last encoder count read from the board
     */
    int16_t lastEncoderCount[8];
    /**
     * @brief  I2C bus to communicate over.
     */
    TwoWire* wire;
    /**
     * @brief  bit mask of which encoders to read from
     */
    uint8_t whichEncodersMask;
    /**
     * @brief  the last time the encoders were read (microseconds)
     */
    unsigned long lastReadMicros[8];
    /**
     * @brief  array of 8 numbers representing the velocity of each encoder (steps per second)
     */
    int16_t encoderVelocity[8];
    /**
     * @brief  after this many milliseconds without an encoder tick velocity is set to zero.
     */
    int16_t encoderSlowestInterval[8];
    /**
     * @brief  enough counts to calculate velocity from
     */
    int16_t encoderEnoughCounts[8];
    /**
     * @brief  array of 8 booleans representing whether the velocity was just calculated
     */
    boolean isVelNewVal[8];
    /**
     * @brief  helper function to write a byte to the board
     */
    inline void write(uint8_t data)
    {
        wire->beginTransmission(address);
        wire->write(data);
        wire->endTransmission();
    }

public:
    /**
     * @brief Constructor for the Byte Sized Encoder Decoder class
     * @param  _wire: I2C bus to communicate over. &Wire or &Wire1
     * @param  _address: I2C address of the Byte Sized Encoder Decoder board (as selected by the jumpers), default is 14, (14-17 are options)
     * @param  _encoderSlowestInterval: after this many milliseconds without a calculation, velocity is recalculated
     * @param  _encoderEnoughCounts: enough counts to calculate velocity
     */
    ByteSizedEncoderDecoder(TwoWire* _wire, uint8_t _address = 14, int16_t _encoderSlowestInterval = 0, int16_t _encoderEnoughCounts = 0)
    {
        wire = _wire;
        address = _address;
        whichEncodersMask = 255;
        memset(encoderCount, 0, 8);
        memset(encoderOverflows, 0, 8);
        memset(lastEncoderCount, 0, 8);
        memset(lastReadMicros, 0, 8);
        memset(encoderVelocity, 0, 8);
        memset(encoderSlowestInterval, _encoderSlowestInterval, 8);
        memset(encoderEnoughCounts, _encoderEnoughCounts, 8);
    }
    /**
     * @brief set the value of encoderSlowestInterval
     * @param  n: encoder number (1-8), or 0 to set all at once
     * @param  interval: after this many milliseconds without a calculation, velocity is recalculated
     */
    void setEncoderSlowestInterval(uint8_t n, int16_t interval)
    {
        if (n > 8) {
            return;
        }
        if (n == 0) {
            for (byte i = 0; i < 8; i++) {
                encoderSlowestInterval[i] = interval;
            }
            return;
        }
        encoderSlowestInterval[n - 1] = interval;
    }
    /**
     * @brief set the value of encoderEnoughCounts
     * @param  n: encoder number (1-8), or 0 to set all at once
     * @param  counts: enough counts to calculate velocity
     */
    void setEncoderEnoughCounts(uint8_t n, int16_t counts)
    {
        if (n > 8) {
            return;
        }
        if (n == 0) {
            for (byte i = 0; i < 8; i++) {
                encoderEnoughCounts[i] = counts;
            }
            return;
        }
        encoderEnoughCounts[n - 1] = counts;
    }

    /**
     * @brief  whether the velocity was just calculated
     * @note this function resets the value to false, so call it just once
     * @param  n: encoder number (1-8)
     * @retval boolean: whether the velocity was just calculated
     */
    boolean isVelNew(uint8_t n)
    {
        if (n > 8 || n < 1) {
            return false;
        }
        boolean temp = isVelNewVal[n - 1];
        isVelNewVal[n - 1] = false;
        return temp;
    }

    /**
     * @brief  sets up the Byte Sized Encoder Decoder board
     * all it really does is tell the board to read from all encoders
     * @param resetEncoders: whether to reset the encoder positions to 0, default is true
     * @note  call this after Wire.begin() has been called
     */
    void begin(boolean resetEncoders = true)
    {
        setWhichEncoders(255);
        if (resetEncoders) {
            resetEncoderPositions();
        }
    }
    /**
     * @brief  reads the encoder positions from the board
     * @note  call this in your loop to update the encoder positions
     */
    void run()
    {
        wire->beginTransmission(address);
        wire->write(whichEncodersMask);
        wire->endTransmission(true);
        delayMicroseconds(50); // time for the board to prepare to respond
        for (byte i = 0; i < 8; i++) {
            if (bitRead(whichEncodersMask, 7 - i) == 0) {
                continue;
            }
            int high = -1;
            int low = -1;
            wire->requestFrom(address, (uint8_t)1);
            if (wire->available())
                high = wire->read();
            wire->requestFrom(address, (uint8_t)1);
            if (wire->available())
                low = wire->read();
            if (high == -1 || low == -1) {
                // we didn't get data
            } else {
                lastEncoderCount[i] = encoderCount[i];
                encoderCount[i] = (((uint16_t)high) << 8 | ((uint16_t)low));
                if (abs(encoderCount[i] - lastEncoderCount[i]) > (1 << 15)) {
                    encoderOverflows[i] += (encoderCount[i] > lastEncoderCount[i]) ? -1 : 1;
                }
                // calculate velocity
                unsigned long mic = micros();
                int32_t hundredMicrosSinceLastRead = (mic - lastReadMicros[i]) / 100; // using a time interval of 100 microseconds (won't overflow int32)
                if (hundredMicrosSinceLastRead > (int32_t)(encoderSlowestInterval[i] * 10) || abs(encoderCount[i] - lastEncoderCount[i]) > encoderEnoughCounts[i]) {
                    lastReadMicros[i] = mic;
                    encoderVelocity[i] = (int32_t)10000 * (encoderCount[i] - lastEncoderCount[i]) / hundredMicrosSinceLastRead;
                    isVelNewVal[i] = true;
                }
            }
        }
    }
    /**
     * @brief  gets the position of an encoder as a 32 bit signed integer (it counts how many times the 16 bit number has overflowed)
     * @param  n: encoder number (1-8), other values will return 0
     * @param  read: whether to read the encoder positions from the board before returning the value, default is false
     * @retval int32_t: the number of steps the encoder has taken
     */
    int32_t getEncoderPosition(uint8_t n, boolean read = false)
    {
        if (read) {
            run();
        }
        if (n > 8 || n < 1) {
            return 0;
        }
        return (int32_t)encoderCount[n - 1] + (int32_t)encoderOverflows[n - 1] * 65536;
    }
    /**
     * @brief  gets the position of an encoder as the 16 bit number that the board returns (it loops around and overflows at 32767 and underflows at -32768)
     * @note  use getEncoderPosition if you want a 32 bit number that includes how many times the number from the board has overflowed
     * @param  n: encoder number (1-8), other values will return 0
     * @param  read: whether to read the encoder positions from the board before returning the value, default is false
     * @retval int16_t: the number of steps the encoder has taken (though it loops around and overflows at 32767 and underflows at -32768)
     */
    int16_t getEncoderPositionWithoutOverflows(uint8_t n, boolean read = false)
    {
        if (read) {
            run();
        }
        if (n > 8 || n < 1) {
            return 0;
        }
        return encoderCount[n - 1];
    }
    /**
     * @brief  gets the velocity of an encoder
     * @param  n: encoder number (1-8), other values will return 0
     * @param  read: whether to read the encoder positions from the board before returning the value, default is false
     * @retval int16_t: the velocity of the encoder (steps per second)
     */
    int16_t getEncoderVelocity(uint8_t n, boolean read = false)
    {
        if (read) {
            run();
        }
        if (n > 8 || n < 1) {
            return 0;
        }
        return encoderVelocity[n - 1];
    }
    /**
     * @brief  resets all encoder positions to 0
     * @param  resetVariables: whether to reset the encoderCount and encoderOverflows variables, default is true
     */
    void resetEncoderPositions(boolean resetVariables = true)
    {
        write(0);
        if (resetVariables) {
            for (byte i = 0; i < 8; i++) {
                encoderCount[i] = 0;
                encoderOverflows[i] = 0;
                lastEncoderCount[i] = 0;
                encoderVelocity[i] = 0;
            }
        }
    }
    /**
     * @brief  set which encoders you want to receive data from
     * @param  mask: 8 bit mask of which encoders to read from
     * 0b00000101 would read from encoders 6 and 8 (aligns with the labels on the board)
     */
    void setWhichEncoders(uint8_t mask)
    {
        if (mask != 0) {
            whichEncodersMask = mask;
            write(mask);
        }
    }
    /**
     * @brief  whether you have set to read data from a specific encoder
     * @param  n: encoder number (1-8)
     * @retval boolean: whether you have set to read data from a specific encoder
     */
    boolean isEncoderActive(uint8_t n)
    {
        if (n > 8 || n < 1) {
            return false;
        }
        return bitRead(whichEncodersMask, 7 - (n - 1));
    }
};
#endif // BYTE_SIZED_ENCODER_DECODER_H
