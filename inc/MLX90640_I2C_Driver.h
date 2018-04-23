#ifndef _MLX90640_I2C_Driver_H_
#define _MLX90640_I2C_Driver_H_

#include <stdint.h>

    void MLX90640_I2CInit(void);
    int MLX90640_I2CRead(uint8_t slaveAddr,unsigned int startAddress, unsigned int nWordsRead, uint16_t *data);
    int MLX90640_I2CWrite(uint8_t slaveAddr,unsigned int writeAddress, uint16_t data);
    void MLX90640_I2CFreqSet(int freq);
#endif