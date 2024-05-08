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
    inline void write(uint8_t data)
    {
        wire->beginTransmission(address);
        wire->write(data);
        wire->endTransmission();
    }
    inline byte bitCount(uint8_t b)
    {
        byte count = 0;
        for (byte i = 0; i < 8; i++) {
            if (bitRead(b, i) == 1) {
                count++;
            }
        }
        return count;
    }

public:
    /**
     * @brief Constructor for the Byte Sized Encoder Decoder class
     * @param  _wire: I2C bus to communicate over. &Wire or &Wire1
     * @param  _address: I2C address of the Byte Sized Encoder Decoder board (as selected by the jumpers), default is 14, (14-17 are options)
     */
    ByteSizedEncoderDecoder(TwoWire* _wire, uint8_t _address = 14)
    {
        wire = _wire;
        address = _address;
        whichEncodersMask = 255;
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
        wire->requestFrom(address, (uint8_t)(2 * bitCount(whichEncodersMask)));
        for (byte i = 0; i < 8; i++) {
            if (bitRead(whichEncodersMask, i) == 0) {
                continue;
            }
            int high = -1;
            int low = -1;
            if (wire->available())
                high = wire->read();
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
            }
        }
    }
    /**
     * @brief  gets the position of an encoder as a 32 bit signed integer (it counts how many times the 16 bit number has overflowed)
     * @param  n: encoder number (1-8), other values will return 0
     * @param  read: whether to read the encoder positions from the board before returning the value, default is false
     * @retval int32_t: the number of steps the encoder has taken
     */
    int32_t getEncoderPositionWithOverflows(uint8_t n, boolean read = false)
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
     * @brief  gets the position of an encoder
     * @param  n: encoder number (1-8), other values will return 0
     * @param  read: whether to read the encoder positions from the board before returning the value, default is false
     * @retval int16_t: the number of steps the encoder has taken (though it loops around and overflows at 32767 and underflows at -32768)
     */
    int16_t getEncoderPosition(uint8_t n, boolean read = false)
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
        return bitRead(whichEncodersMask, n - 1);
    }
};
#endif // BYTE_SIZED_ENCODER_DECODER_H
