/**
 * @copyright (C) 2018 Thomas Fischl, https://www.fischl.de
 *
 * Driver for USB2FIR (https://www.fischl.de/usb2fir/), an USB interface
 * board for MLX90640. USB control transfer is used to access the MLX90640
 * memory (EEPROM, IR data). Bulk-transfer mode for fast infrared data
 * transmission is not yet supported by this driver.
 *
 * Example gcc command line to integrate this driver in your project:
 * gcc -I mlx90640-library/headers/ mlx90640-library/functions/MLX90640_USB2FIR_Driver.cpp mlx90640-library/functions/MLX90640_API.cpp main.cpp -lm -lusb-1.0
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
#include <fcntl.h>  
#include <unistd.h> 
#include <stdio.h>
#include <sys/ioctl.h>

#include <libusb-1.0/libusb.h>

#include "MLX90640_I2C_Driver.h"

#define VENDOR_ID  0x04D8
#define PRODUCT_ID 0xEE7D 

#define CMD_GET_CAPABILITY   0
#define CMD_ECHO             1
#define CMD_START_BOOTLOADER 2
#define CMD_READ_MEMORY      3
#define CMD_WRITE_MEMORY     4
#define CMD_GET_STATUS       5

#define TIMEOUT 1000

static struct libusb_device_handle *devh = NULL;


void MLX90640_I2CInit()
{   

    int rc;

    rc = libusb_init(NULL);
    if (rc < 0) {
        fprintf(stderr, "Error initializing libusb: %s\n", libusb_error_name(rc));
        return;
    }


    devh = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
    if (!devh) {
        fprintf(stderr, "Error finding USB device\n");
        return;
    }


    libusb_claim_interface (devh, 0);

}
    
int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{

    int rc;
    int i, cnt;
    uint8_t inbuf[1664] = {0};

    rc = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, CMD_READ_MEMORY, slaveAddr, startAddress, inbuf, nMemAddressRead * 2, TIMEOUT);

    if (rc < 0) {
        fprintf(stderr, "Error during control transfer: %s\n",
                libusb_error_name(rc));
        return -1;
    }

    for (cnt = 0; cnt < nMemAddressRead; cnt++)
    {
        i = cnt << 1;
        data[cnt] = (uint16_t)inbuf[i]*256 + (uint16_t)inbuf[i+1];
    }

    return 0;
  
} 

void MLX90640_I2CFreqSet(int freq)
{
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
 
    int rc;
    uint8_t outbuf[2];
    uint16_t dataCheck;

    outbuf[0] = data >> 8;
    outbuf[1] = data & 0x00FF;
    
    rc = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, CMD_WRITE_MEMORY, slaveAddr, writeAddress, outbuf, 2, TIMEOUT);

    if (rc < 0) {
        fprintf(stderr, "Error during control transfer: %s\n",
                libusb_error_name(rc));
        return -1;
    }

    MLX90640_I2CRead(slaveAddr,writeAddress,1, &dataCheck);
    
    if ( dataCheck != data)
    {
        return -2;
    }    

    return 0;
}

