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
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/i2c-dev.h>

#ifndef I2C_FUNC_I2C
#include <linux/i2c.h>
#define I2C_MSG_FMT __u8
#endif

#include <sys/ioctl.h>

int i2c_fd = 0;
const char *i2c_device = "/dev/i2c-1";

void MLX90640_I2CInit()
{
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    if(!i2c_fd){
        i2c_fd = open(i2c_device, O_RDWR);
    }

    int result;
    char cmd[2] = {(char)(startAddress >> 8), (char)(startAddress & 0xFF)};
    char buf[1664];
    uint16_t *p = data;
    struct i2c_msg i2c_messages[2];
    struct i2c_rdwr_ioctl_data i2c_messageset[1];

    i2c_messages[0].addr = slaveAddr;
    i2c_messages[0].flags = 0;
    i2c_messages[0].len = 2;
    i2c_messages[0].buf = (I2C_MSG_FMT*)cmd;

    i2c_messages[1].addr = slaveAddr;
    i2c_messages[1].flags = I2C_M_RD | I2C_M_NOSTART;
    i2c_messages[1].len = nMemAddressRead * 2;
    i2c_messages[1].buf = (I2C_MSG_FMT*)buf;

    //result = write(i2c_fd, cmd, 3);
    //result = read(i2c_fd, buf, nMemAddressRead*2);
    i2c_messageset[0].msgs = i2c_messages;
    i2c_messageset[0].nmsgs = 2;

    memset(buf, 0, nMemAddressRead * 2);

    if (ioctl(i2c_fd, I2C_RDWR, &i2c_messageset) < 0) {
#ifdef DEBUG
        printf("I2C Read Error!\n");
#endif
        return -1;
    }

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
    char cmd[4] = {(char)(writeAddress >> 8), (char)(writeAddress & 0x00FF), (char)(data >> 8), (char)(data & 0x00FF)};
    int result;

    struct i2c_msg i2c_messages[1];
    struct i2c_rdwr_ioctl_data i2c_messageset[1];

    i2c_messages[0].addr = slaveAddr;
    i2c_messages[0].flags = 0;
    i2c_messages[0].len = 4;
    i2c_messages[0].buf = (I2C_MSG_FMT*)cmd;

    i2c_messageset[0].msgs = i2c_messages;
    i2c_messageset[0].nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &i2c_messageset) < 0) {
#ifdef DEBUG
        printf("I2C Write Error!\n");
#endif
        return -1;
    }

    return 0;
}
