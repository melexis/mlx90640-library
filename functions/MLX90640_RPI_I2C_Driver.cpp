/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "MLX90640_I2C_Driver.h"
#include <iostream>
#include <bcm2835.h>

int init = 0;

void MLX90640_I2CInit()
{
    
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    if(!init){
        bcm2835_init();
    	bcm2835_i2c_begin();
	bcm2835_i2c_set_baudrate(400000);
	init = 1;
    }

    int result;

    char cmd[2] = {(char)(startAddress >> 8), (char)(startAddress & 0xFF)};
    

    bcm2835_i2c_setSlaveAddress(slaveAddr);

    char buf[1664];
    uint16_t *p = data;

    result = bcm2835_i2c_write_read_rs(cmd, 2, buf, nMemAddressRead*2);

    for(int count = 0; count < nMemAddressRead; count++){
	int i = count << 1;
    	*p++ = ((uint16_t)buf[i] << 8) | buf[i+1];
    }
    return 0;
} 

void MLX90640_I2CFreqSet(int freq)
{
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    int result;
    char cmd[4] = {(char)(writeAddress >> 8), (char)(writeAddress & 0x00FF), (char)(data >> 8), (char)(data & 0x00FF)};
    result = bcm2835_i2c_write(cmd, 4);
    return 0;
}
