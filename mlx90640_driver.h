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

#ifndef _mlx90640_driver_H_
#define _mlx90640_driver_H_

#include <stdint.h>
#include "mlx90640_driver.h"
#include <math.h>
#include <stdio.h>
#include <chrono>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include "testpic.h"
#include "mywidget.h"

#define SCALEALPHA 0.000001
#define I2C_MSG_FMT char
#ifndef I2C_FUNC_I2C
#include <linux/i2c.h>
#define I2C_MSG_FMT __u8

#endif

#include <sys/ioctl.h>

class MLX90640 {

public:

    //

    float mlx90640To[768];
    static uint16_t eeMLX90640[832];

    // main funcs
    void run();
    void start();
    void registerCallback(MyWidget* mywidget);
    void stop();
    void MLX90640_I2CInit(void);
    
    // Original driver apis
    int MLX90640_DumpEE(uint8_t slaveAddr, uint16_t *eeData);
    void MLX90640_SetEmissivity(float em);
    int MLX90640_GetFrameData(uint8_t slaveAddr, uint16_t *frameData);
    int MLX90640_ExtractParameters(uint16_t *eeData, MLX90640 *mlx90640);
    float MLX90640_GetVdd(uint16_t *frameData, const MLX90640 *params);
    float MLX90640_GetTa(uint16_t *frameData, const MLX90640 *params);
    void MLX90640_GetImage(uint16_t *frameData, const MLX90640 *params, float *result);
    void MLX90640_CalculateTo(uint16_t *frameData, const MLX90640 *params, float emissivity, float tr, float *result);
    int MLX90640_SetResolution(uint8_t slaveAddr, uint8_t resolution);
    int MLX90640_GetCurResolution(uint8_t slaveAddr);
    int MLX90640_SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate);   
    int MLX90640_GetRefreshRate(uint8_t slaveAddr);  
    int MLX90640_GetSubPageNumber(uint16_t *frameData);
    int MLX90640_GetCurMode(uint8_t slaveAddr); 
    int MLX90640_SetInterleavedMode(uint8_t slaveAddr);
    int MLX90640_SetChessMode(uint8_t slaveAddr);
    void MLX90640_BadPixelsCorrection(uint16_t *pixels, float *to, int mode, MLX90640 *params);

    // Added driver apis
    int MLX90640_SetDeviceMode(uint8_t slaveAddr, uint8_t deviceMode);
    int MLX90640_SetSubPageRepeat(uint8_t slaveAddr, uint8_t subPageRepeat);
    int MLX90640_SetSubPage(uint8_t slaveAddr, uint8_t subPage);
    int MLX90640_CheckInterrupt(uint8_t slaveAddr);
    void MLX90640_StartMeasurement(uint8_t slaveAddr, uint8_t subPage);
    int MLX90640_GetData(uint8_t slaveAddr, uint16_t *frameData);
    int MLX90640_InterpolateOutliers(uint16_t *frameData, uint16_t *eepromData);
    
    void ExtractVDDParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractPTATParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractGainParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractTgcParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractResolutionParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractKsTaParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractKsToParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractAlphaParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractOffsetParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractKtaPixelParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractKvPixelParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractCPParameters(uint16_t *eeData, MLX90640 *mlx90640);
    void ExtractCILCParameters(uint16_t *eeData, MLX90640 *mlx90640);
    int ExtractDeviatingPixels(uint16_t *eeData, MLX90640 *mlx90640);
    int CheckAdjacentPixels(uint16_t pix1, uint16_t pix2);  
    float GetMedian(float *values, int n);
    int IsPixelBad(uint16_t pixel,MLX90640 *params);


private:  
    //
    int state;
    float eTa;
    float emissivity = 0.9;
    uint16_t frame[834];
    static float image[768];
    static uint16_t data[768*sizeof(float)];

    // I2C paras
    int i2c_fd = 0;
    const char *i2c_device = "/dev/i2c-1"; 

    // Driver api paras
    int16_t kVdd;
    int16_t vdd25;
    float KvPTAT;
    float KtPTAT;
    uint16_t vPTAT25;
    float alphaPTAT;
    int16_t gainEE;
    float tgc;
    float cpKv;
    float cpKta;
    uint8_t resolutionEE;
    uint8_t calibrationModeEE;
    float KsTa;
    float ksTo[5];
    int16_t ct[5];
    uint16_t alpha[768];    
    uint8_t alphaScale;
    int16_t offset[768];    
    int8_t kta[768];
    uint8_t ktaScale;    
    int8_t kv[768];
    uint8_t kvScale;
    float cpAlpha[2];
    int16_t cpOffset[2];
    float ilChessC[3]; 
    uint16_t brokenPixels[5];
    uint16_t outlierPixels[5];    

    // I2C funcs
    
    int MLX90640_I2CRead(uint8_t slaveAddr,uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data);
    int MLX90640_I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data);
    void MLX90640_I2CFreqSet(int freq);

    // exec
    MyWidget* mlx90640_callback = nullptr;
    int running = 0;
    std::thread* daqThread = nullptr;
    static void exec(MLX90640* mlx90640) {
        mlx90640->run();
    }
};

#endif