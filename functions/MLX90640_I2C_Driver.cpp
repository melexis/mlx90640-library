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
#include "mbed.h"
#include "MLX90640_I2C_Driver.h"

I2C i2c(p9, p10);

void MLX90640_I2CInit()
{   
    i2c.stop();
}

int MLX90640_I2CGeneralReset(void)
{    
    int ack;
    char cmd[2] = {0,0};
    
    cmd[0] = 0x00;
    cmd[1] = 0x06;    

    i2c.stop();
    wait_us(5);    
    ack = i2c.write(cmd[0], &cmd[1], 1, 0);
    
    if (ack != 0x00)
    {
        return -1;
    }         
    i2c.stop();   
    
    wait_us(50);    
    
    return 0;
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    uint8_t sa;                           
    int ack = 0;                               
    int cnt = 0;
    int i = 0;
    char cmd[2] = {0,0};
    uint16_t *p;
    uint8_t *byte_data = (uint8_t*) data;
    
    p = data;
    sa = (slaveAddr << 1);
    cmd[0] = startAddress >> 8;
    cmd[1] = startAddress & 0x00FF;
    
    i2c.stop();
    wait_us(5);    
    ack = i2c.write(sa, cmd, 2, 1);
    
    if (ack != 0x00)
    {
        return -1;
    }
             
    sa = sa | 0x01;
    ack = i2c.read(sa, byte_data, 2*nMemAddressRead, 0);
    
    if (ack != 0x00)
    {
        return -1; 
    }          
    i2c.stop();   
    
    for(cnt=0; cnt < nMemAddressRead; cnt++)
    {
        i = cnt << 1;
        uint16_t tmp = byte_data[i+1];
        tmp = (tmp << 8) | byte_data[i];
        *p++ = tmp;
    }
    
    return 0;   
} 

void MLX90640_I2CFreqSet(int freq)
{
    i2c.frequency(1000*freq);
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    uint8_t sa;
    int ack = 0;
    char cmd[4] = {0,0,0,0};
    static uint16_t dataCheck;
    

    sa = (slaveAddr << 1);
    cmd[0] = writeAddress >> 8;
    cmd[1] = writeAddress & 0x00FF;
    cmd[2] = data >> 8;
    cmd[3] = data & 0x00FF;

    i2c.stop();
    wait_us(5);    
    ack = i2c.write(sa, cmd, 4, 0);
    
    if (ack != 0x00)
    {
        return -1;
    }         
    i2c.stop();   
    
    MLX90640_I2CRead(slaveAddr,writeAddress,1, &dataCheck);
    
    if ( dataCheck != data)
    {
        return -2;
    }    
    
    return 0;
}

