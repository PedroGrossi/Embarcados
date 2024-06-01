#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

void I2C0_sendSingleByte(uint8_t slave_addr, char data);
void I2C0_sendMultipleBytes(uint8_t slave_addr, uint8_t numOfBytes, char by[]);
void I2C7_sendSingleByte(uint8_t slave_addr, char data);
void I2C7_sendMultipleBytes(uint8_t slave_addr, uint8_t numOfBytes, char by[]);

#endif 
