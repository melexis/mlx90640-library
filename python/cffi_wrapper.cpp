#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <math.h>
#include "../headers/MLX90640_API.h"
#include "../headers/MLX90640_I2C_Driver.h"

extern "C" int I2CRead(uint8_t slaveAddr,uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data) {
    return MLX90640_I2CRead(slaveAddr, startAddress, nMemAddressRead, data);
}

extern "C" int I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data) {
    return MLX90640_I2CWrite(slaveAddr, writeAddress, data);
}

extern "C" int DumpEE(uint8_t slaveAddr, uint16_t *eeData) {
    return MLX90640_DumpEE(slaveAddr, eeData);
}

extern "C" int GetFrameData(uint8_t slaveAddr, uint16_t *frameData) {
    return MLX90640_GetFrameData(slaveAddr, frameData);
}

extern "C" int ExtractParameters(uint16_t *eeData, paramsMLX90640 *mlx90640) {
    return MLX90640_ExtractParameters(eeData, mlx90640);
}

extern "C" float GetVdd(uint16_t *frameData, const paramsMLX90640 *params) {
    return MLX90640_GetVdd(frameData, params);
}

extern "C" float GetTa(uint16_t *frameData, const paramsMLX90640 *params) {
    return MLX90640_GetTa(frameData, params);
}

extern "C" void GetImage(uint16_t *frameData, const paramsMLX90640 *params, float *result) {
    return MLX90640_GetImage(frameData, params, result);
}

extern "C" void CalculateTo(uint16_t *frameData, const paramsMLX90640 *params, float emissivity, float tr, float *result) {
    return MLX90640_CalculateTo(frameData, params, emissivity, tr, result);
}

extern "C" int SetResolution(uint8_t slaveAddr, uint8_t resolution) {
    return MLX90640_SetResolution(slaveAddr, resolution);
}

extern "C" int GetCurResolution(uint8_t slaveAddr) {
    return MLX90640_GetCurResolution(slaveAddr);
}

extern "C" int SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate) {   
    return MLX90640_SetRefreshRate(slaveAddr, refreshRate);   
}

extern "C" int GetRefreshRate(uint8_t slaveAddr) {  
    return MLX90640_GetRefreshRate(slaveAddr);  
}

extern "C" int GetSubPageNumber(uint16_t *frameData) {
    return MLX90640_GetSubPageNumber(frameData);
}

extern "C" int GetCurMode(uint8_t slaveAddr) { 
    return MLX90640_GetCurMode(slaveAddr); 
}

extern "C" int SetInterleavedMode(uint8_t slaveAddr) {
    return MLX90640_SetInterleavedMode(slaveAddr);
}

extern "C" int SetChessMode(uint8_t slaveAddr) {
    return MLX90640_SetChessMode(slaveAddr);
}

extern "C" void BadPixelsCorrection(uint16_t *pixels, float *to, int mode, paramsMLX90640 *params) {
    return MLX90640_BadPixelsCorrection(pixels, to, mode, params);
}

