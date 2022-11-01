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
#include <MLX90640_I2C_Driver.h>
#include <MLX90640_API.h>
#include <math.h>

static void ExtractVDDParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractPTATParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractGainParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractTgcParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractResolutionParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractKsTaParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractKsToParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractAlphaParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractOffsetParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractKtaPixelParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractKvPixelParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractCPParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void ExtractCILCParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static int ExtractDeviatingPixels(uint16_t *eeData, paramsMLX90640 *mlx90640);
static int CheckAdjacentPixels(uint16_t pix1, uint16_t pix2);  
static float GetMedian(float *values, int n);
static int IsPixelBad(uint16_t pixel,paramsMLX90640 *params);
static int ValidateFrameData(uint16_t *frameData);
static int ValidateAuxData(uint16_t *auxData);
  
int MLX90640_DumpEE(uint8_t slaveAddr, uint16_t *eeData)
{
     return MLX90640_I2CRead(slaveAddr, MLX90640_EEPROM_START_ADDRESS, MLX90640_EEPROM_DUMP_NUM, eeData);
}

int MLX90640_SynchFrame(uint8_t slaveAddr)
{
    uint16_t dataReady = 0;
    uint16_t statusRegister;
    int error = 1;
    
    error = MLX90640_I2CWrite(slaveAddr, MLX90640_STATUS_REG, MLX90640_INIT_STATUS_VALUE);
    if(error == -MLX90640_I2C_NACK_ERROR)
    {
        return error;
    }
    
    while(dataReady == 0)
    {
        error = MLX90640_I2CRead(slaveAddr, MLX90640_STATUS_REG, 1, &statusRegister);
        if(error != MLX90640_NO_ERROR)
        {
            return error;
        }    
        //dataReady = statusRegister & 0x0008;
        dataReady = MLX90640_GET_DATA_READY(statusRegister); 
    }     
    
   return MLX90640_NO_ERROR;   
}

int MLX90640_TriggerMeasurement(uint8_t slaveAddr)
{
    int error = 1;
    uint16_t ctrlReg;
    
    error = MLX90640_I2CRead(slaveAddr, MLX90640_CTRL_REG, 1, &ctrlReg);
    
    if ( error != MLX90640_NO_ERROR) 
    {
        return error;
    }    
                                                
    ctrlReg |= MLX90640_CTRL_TRIG_READY_MASK;
    error = MLX90640_I2CWrite(slaveAddr, MLX90640_CTRL_REG, ctrlReg);
    
    if ( error != MLX90640_NO_ERROR)
    {
        return error;
    }    
    
    error = MLX90640_I2CGeneralReset();
    
    if ( error != MLX90640_NO_ERROR)
    {
        return error;
    }    
    
    error = MLX90640_I2CRead(slaveAddr, MLX90640_CTRL_REG, 1, &ctrlReg);
    
    if ( error != MLX90640_NO_ERROR)
    {
        return error;
    }    
    
    if ((ctrlReg & MLX90640_CTRL_TRIG_READY_MASK) != 0)
    {
        return -MLX90640_MEAS_TRIGGER_ERROR;
    }
    
    return MLX90640_NO_ERROR;    
}
    
int MLX90640_GetFrameData(uint8_t slaveAddr, uint16_t *frameData)
{
    uint16_t dataReady = 0;
    uint16_t controlRegister1;
    uint16_t statusRegister;
    int error = 1;
    uint16_t data[64];
    uint8_t cnt = 0;
    
    while(dataReady == 0)
    {
        error = MLX90640_I2CRead(slaveAddr, MLX90640_STATUS_REG, 1, &statusRegister);
        if(error != MLX90640_NO_ERROR)
        {
            return error;
        }    
        //dataReady = statusRegister & 0x0008;
        dataReady = MLX90640_GET_DATA_READY(statusRegister); 
    }      
    
    error = MLX90640_I2CWrite(slaveAddr, MLX90640_STATUS_REG, MLX90640_INIT_STATUS_VALUE);
    if(error == -MLX90640_I2C_NACK_ERROR)
    {
        return error;
    }
                     
    error = MLX90640_I2CRead(slaveAddr, MLX90640_PIXEL_DATA_START_ADDRESS, MLX90640_PIXEL_NUM, frameData); 
    if(error != MLX90640_NO_ERROR)
    {
        return error;
    }                       
    
    error = MLX90640_I2CRead(slaveAddr, MLX90640_AUX_DATA_START_ADDRESS, MLX90640_AUX_NUM, data); 
    if(error != MLX90640_NO_ERROR)
    {
        return error;
    }     
        
    error = MLX90640_I2CRead(slaveAddr, MLX90640_CTRL_REG, 1, &controlRegister1);
    frameData[832] = controlRegister1;
    //frameData[833] = statusRegister & 0x0001;
    frameData[833] = MLX90640_GET_FRAME(statusRegister);
    
    if(error != MLX90640_NO_ERROR)
    {
        return error;
    }
    
    error = ValidateAuxData(data);
    if(error == MLX90640_NO_ERROR)
    {
        for(cnt=0; cnt<MLX90640_AUX_NUM; cnt++)
        {
            frameData[cnt+MLX90640_PIXEL_NUM] = data[cnt];
        }
    }        
    
    error = ValidateFrameData(frameData);
    if (error != MLX90640_NO_ERROR)
    {
        return error;
    }
    
    return frameData[833];    
}

static int ValidateFrameData(uint16_t *frameData)
{
    uint8_t line = 0;
    
    for(int i=0; i<MLX90640_PIXEL_NUM; i+=MLX90640_LINE_SIZE)
    {
        if((frameData[i] == 0x7FFF) && (line%2 == frameData[833])) return -MLX90640_FRAME_DATA_ERROR;
        line = line + 1;
    }    
        
    return MLX90640_NO_ERROR;    
}

static int ValidateAuxData(uint16_t *auxData)
{
    
    if(auxData[0] == 0x7FFF) return -MLX90640_FRAME_DATA_ERROR;    
    
    for(int i=8; i<19; i++)
    {
        if(auxData[i] == 0x7FFF) return -MLX90640_FRAME_DATA_ERROR;
    }
    
    for(int i=20; i<23; i++)
    {
        if(auxData[i] == 0x7FFF) return -MLX90640_FRAME_DATA_ERROR;
    }
    
    for(int i=24; i<33; i++)
    {
        if(auxData[i] == 0x7FFF) return -MLX90640_FRAME_DATA_ERROR;
    }
    
    for(int i=40; i<51; i++)
    {
        if(auxData[i] == 0x7FFF) return -MLX90640_FRAME_DATA_ERROR;
    }
    
    for(int i=52; i<55; i++)
    {
        if(auxData[i] == 0x7FFF) return -MLX90640_FRAME_DATA_ERROR;
    }
    
    for(int i=56; i<64; i++)
    {
        if(auxData[i] == 0x7FFF) return -MLX90640_FRAME_DATA_ERROR;
    }
    
    return MLX90640_NO_ERROR;
    
}
    
int MLX90640_ExtractParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    int error = 0;
    
    ExtractVDDParameters(eeData, mlx90640);
    ExtractPTATParameters(eeData, mlx90640);
    ExtractGainParameters(eeData, mlx90640);
    ExtractTgcParameters(eeData, mlx90640);
    ExtractResolutionParameters(eeData, mlx90640);
    ExtractKsTaParameters(eeData, mlx90640);
    ExtractKsToParameters(eeData, mlx90640);
    ExtractCPParameters(eeData, mlx90640);
    ExtractAlphaParameters(eeData, mlx90640);
    ExtractOffsetParameters(eeData, mlx90640);
    ExtractKtaPixelParameters(eeData, mlx90640);
    ExtractKvPixelParameters(eeData, mlx90640);
    ExtractCILCParameters(eeData, mlx90640);
    error = ExtractDeviatingPixels(eeData, mlx90640);  
    
    return error;

}

//------------------------------------------------------------------------------

int MLX90640_SetResolution(uint8_t slaveAddr, uint8_t resolution)
{
    uint16_t controlRegister1;
    uint16_t value;
    int error;
    
    //value = (resolution & 0x03) << 10;
    value = ((uint16_t)resolution << MLX90640_CTRL_RESOLUTION_SHIFT);
    value &= ~MLX90640_CTRL_RESOLUTION_MASK;
    
    error = MLX90640_I2CRead(slaveAddr, MLX90640_CTRL_REG, 1, &controlRegister1);
    
    if(error == MLX90640_NO_ERROR)
    {
        value = (controlRegister1 & MLX90640_CTRL_RESOLUTION_MASK) | value;
        error = MLX90640_I2CWrite(slaveAddr, MLX90640_CTRL_REG, value);        
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90640_GetCurResolution(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int resolutionRAM;
    int error;
    
    error = MLX90640_I2CRead(slaveAddr, MLX90640_CTRL_REG, 1, &controlRegister1);
    if(error != MLX90640_NO_ERROR)
    {
        return error;
    }    
    resolutionRAM = (controlRegister1 & ~MLX90640_CTRL_RESOLUTION_MASK) >> MLX90640_CTRL_RESOLUTION_SHIFT;
    
    return resolutionRAM; 
}

//------------------------------------------------------------------------------

int MLX90640_SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate)
{
    uint16_t controlRegister1;
    uint16_t value;
    int error;
    
    //value = (refreshRate & 0x07)<<7;
    value = ((uint16_t)refreshRate << MLX90640_CTRL_REFRESH_SHIFT);
    value &= ~MLX90640_CTRL_REFRESH_MASK;
    
    error = MLX90640_I2CRead(slaveAddr, MLX90640_CTRL_REG, 1, &controlRegister1);
    if(error == MLX90640_NO_ERROR)
    {
        value = (controlRegister1 & MLX90640_CTRL_REFRESH_MASK) | value;
        error = MLX90640_I2CWrite(slaveAddr, MLX90640_CTRL_REG, value);
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90640_GetRefreshRate(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int refreshRate;
    int error;
    
    error = MLX90640_I2CRead(slaveAddr, MLX90640_CTRL_REG, 1, &controlRegister1);
    if(error != MLX90640_NO_ERROR)
    {
        return error;
    }    
    refreshRate = (controlRegister1 & ~MLX90640_CTRL_REFRESH_MASK) >> MLX90640_CTRL_REFRESH_SHIFT;
    
    return refreshRate;
}

//------------------------------------------------------------------------------

int MLX90640_SetInterleavedMode(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    uint16_t value;
    int error;
    
    error = MLX90640_I2CRead(slaveAddr, MLX90640_CTRL_REG, 1, &controlRegister1);
    
    if(error == 0)
    {
        value = (controlRegister1 & ~MLX90640_CTRL_MEAS_MODE_MASK);
        error = MLX90640_I2CWrite(slaveAddr, MLX90640_CTRL_REG, value);        
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90640_SetChessMode(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    uint16_t value;
    int error;
        
    error = MLX90640_I2CRead(slaveAddr, MLX90640_CTRL_REG, 1, &controlRegister1);
    
    if(error == 0)
    {
        value = (controlRegister1 | MLX90640_CTRL_MEAS_MODE_MASK);
        error = MLX90640_I2CWrite(slaveAddr, MLX90640_CTRL_REG, value);        
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90640_GetCurMode(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int modeRAM;
    int error;
    
    error = MLX90640_I2CRead(slaveAddr, MLX90640_CTRL_REG, 1, &controlRegister1);
    if(error != 0)
    {
        return error;
    }    
    modeRAM = (controlRegister1 & MLX90640_CTRL_MEAS_MODE_MASK) >> MLX90640_CTRL_MEAS_MODE_SHIFT;
    
    return modeRAM; 
}

//------------------------------------------------------------------------------

void MLX90640_CalculateTo(uint16_t *frameData, const paramsMLX90640 *params, float emissivity, float tr, float *result)
{
    float vdd;
    float ta;
    float ta4;
    float tr4;
    float taTr;
    float gain;
    float irDataCP[2];
    float irData;
    float alphaCompensated;
    uint8_t mode;
    int8_t ilPattern;
    int8_t chessPattern;
    int8_t pattern;
    int8_t conversionPattern;
    float Sx;
    float To;
    float alphaCorrR[4];
    int8_t range;
    uint16_t subPage;
    float ktaScale;
    float kvScale;
    float alphaScale;
    float kta;
    float kv;
    
    subPage = frameData[833];
    vdd = MLX90640_GetVdd(frameData, params);
    ta = MLX90640_GetTa(frameData, params);
    
    ta4 = (ta + 273.15);
    ta4 = ta4 * ta4;
    ta4 = ta4 * ta4;
    tr4 = (tr + 273.15);
    tr4 = tr4 * tr4;
    tr4 = tr4 * tr4;
    taTr = tr4 - (tr4-ta4)/emissivity;
    
    ktaScale = POW2(params->ktaScale);
    kvScale = POW2(params->kvScale);
    alphaScale = POW2(params->alphaScale);
    
    alphaCorrR[0] = 1 / (1 + params->ksTo[0] * 40);
    alphaCorrR[1] = 1 ;
    alphaCorrR[2] = (1 + params->ksTo[1] * params->ct[2]);
    alphaCorrR[3] = alphaCorrR[2] * (1 + params->ksTo[2] * (params->ct[3] - params->ct[2]));
    
//------------------------- Gain calculation -----------------------------------    
    
    gain = (float)params->gainEE / (int16_t)frameData[778]; 
  
//------------------------- To calculation -------------------------------------    
    mode = (frameData[832] & MLX90640_CTRL_MEAS_MODE_MASK) >> 5;
    
    irDataCP[0] = (int16_t)frameData[776] * gain;
    irDataCP[1] = (int16_t)frameData[808] * gain;
    
    irDataCP[0] = irDataCP[0] - params->cpOffset[0] * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - 3.3));
    if( mode ==  params->calibrationModeEE)
    {
        irDataCP[1] = irDataCP[1] - params->cpOffset[1] * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - 3.3));
    }
    else
    {
      irDataCP[1] = irDataCP[1] - (params->cpOffset[1] + params->ilChessC[0]) * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - 3.3));
    }

    for( int pixelNumber = 0; pixelNumber < 768; pixelNumber++)
    {
        ilPattern = pixelNumber / 32 - (pixelNumber / 64) * 2; 
        chessPattern = ilPattern ^ (pixelNumber - (pixelNumber/2)*2); 
        conversionPattern = ((pixelNumber + 2) / 4 - (pixelNumber + 3) / 4 + (pixelNumber + 1) / 4 - pixelNumber / 4) * (1 - 2 * ilPattern);
        
        if(mode == 0)
        {
          pattern = ilPattern; 
        }
        else 
        {
          pattern = chessPattern; 
        }               
        
        if(pattern == frameData[833])
        {    
            irData = (int16_t)frameData[pixelNumber] * gain;
            
            kta = params->kta[pixelNumber]/ktaScale;
            kv = params->kv[pixelNumber]/kvScale;
            irData = irData - params->offset[pixelNumber]*(1 + kta*(ta - 25))*(1 + kv*(vdd - 3.3));
            
            if(mode !=  params->calibrationModeEE)
            {
              irData = irData + params->ilChessC[2] * (2 * ilPattern - 1) - params->ilChessC[1] * conversionPattern; 
            }                       
    
            irData = irData - params->tgc * irDataCP[subPage];
            irData = irData / emissivity;
            
            alphaCompensated = SCALEALPHA*alphaScale/params->alpha[pixelNumber];
            alphaCompensated = alphaCompensated*(1 + params->KsTa * (ta - 25));
                        
            Sx = alphaCompensated * alphaCompensated * alphaCompensated * (irData + alphaCompensated * taTr);
            Sx = sqrt(sqrt(Sx)) * params->ksTo[1];            
            
            To = sqrt(sqrt(irData/(alphaCompensated * (1 - params->ksTo[1] * 273.15) + Sx) + taTr)) - 273.15;                     
                    
            if(To < params->ct[1])
            {
                range = 0;
            }
            else if(To < params->ct[2])   
            {
                range = 1;            
            }   
            else if(To < params->ct[3])
            {
                range = 2;            
            }
            else
            {
                range = 3;            
            }      
            
            To = sqrt(sqrt(irData / (alphaCompensated * alphaCorrR[range] * (1 + params->ksTo[range] * (To - params->ct[range]))) + taTr)) - 273.15;
                        
            result[pixelNumber] = To;
        }
    }
}

//------------------------------------------------------------------------------

void MLX90640_GetImage(uint16_t *frameData, const paramsMLX90640 *params, float *result)
{
    float vdd;
    float ta;
    float gain;
    float irDataCP[2];
    float irData;
    float alphaCompensated;
    uint8_t mode;
    int8_t ilPattern;
    int8_t chessPattern;
    int8_t pattern;
    int8_t conversionPattern;
    float image;
    uint16_t subPage;
    float ktaScale;
    float kvScale;
    float kta;
    float kv;
    
    subPage = frameData[833];
    vdd = MLX90640_GetVdd(frameData, params);
    ta = MLX90640_GetTa(frameData, params);
    
    ktaScale = POW2(params->ktaScale);
    kvScale = POW2(params->kvScale);
    
//------------------------- Gain calculation -----------------------------------    
    
    gain = (float)params->gainEE / (int16_t)frameData[778]; 
  
//------------------------- Image calculation -------------------------------------    
    
    mode = (frameData[832] & MLX90640_CTRL_MEAS_MODE_MASK) >> 5;
    
    irDataCP[0] = (int16_t)frameData[776] * gain;
    irDataCP[1] = (int16_t)frameData[808] * gain;
    
    irDataCP[0] = irDataCP[0] - params->cpOffset[0] * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - 3.3));
    if( mode ==  params->calibrationModeEE)
    {
        irDataCP[1] = irDataCP[1] - params->cpOffset[1] * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - 3.3));
    }
    else
    {
      irDataCP[1] = irDataCP[1] - (params->cpOffset[1] + params->ilChessC[0]) * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - 3.3));
    }

    for( int pixelNumber = 0; pixelNumber < 768; pixelNumber++)
    {
        ilPattern = pixelNumber / 32 - (pixelNumber / 64) * 2; 
        chessPattern = ilPattern ^ (pixelNumber - (pixelNumber/2)*2); 
        conversionPattern = ((pixelNumber + 2) / 4 - (pixelNumber + 3) / 4 + (pixelNumber + 1) / 4 - pixelNumber / 4) * (1 - 2 * ilPattern);
        
        if(mode == 0)
        {
          pattern = ilPattern; 
        }
        else 
        {
          pattern = chessPattern; 
        }
        
        if(pattern == frameData[833])
        {    
            irData = (int16_t)frameData[pixelNumber] * gain;
            
            kta = params->kta[pixelNumber]/ktaScale;
            kv = params->kv[pixelNumber]/kvScale;
            irData = irData - params->offset[pixelNumber]*(1 + kta*(ta - 25))*(1 + kv*(vdd - 3.3));

            if(mode !=  params->calibrationModeEE)
            {
              irData = irData + params->ilChessC[2] * (2 * ilPattern - 1) - params->ilChessC[1] * conversionPattern; 
            }
            
            irData = irData - params->tgc * irDataCP[subPage];
                        
            alphaCompensated = params->alpha[pixelNumber];
            
            image = irData*alphaCompensated;
            
            result[pixelNumber] = image;
        }
    }
}

//------------------------------------------------------------------------------

float MLX90640_GetVdd(uint16_t *frameData, const paramsMLX90640 *params)
{
    float vdd;
    float resolutionCorrection;

    uint16_t resolutionRAM;  
    
    resolutionRAM = (frameData[832] & ~MLX90640_CTRL_RESOLUTION_MASK) >> MLX90640_CTRL_RESOLUTION_SHIFT;   
    resolutionCorrection = POW2(params->resolutionEE) / POW2(resolutionRAM);
    vdd = (resolutionCorrection * (int16_t)frameData[810] - params->vdd25) / params->kVdd + 3.3;
    
    return vdd;
}

//------------------------------------------------------------------------------

float MLX90640_GetTa(uint16_t *frameData, const paramsMLX90640 *params)
{
    int16_t ptat;
    float ptatArt;
    float vdd;
    float ta;
    
    vdd = MLX90640_GetVdd(frameData, params);
    
    ptat = (int16_t)frameData[800];
    
    ptatArt = (ptat / (ptat * params->alphaPTAT + (int16_t)frameData[768])) * POW2(18);
    
    ta = (ptatArt / (1 + params->KvPTAT * (vdd - 3.3)) - params->vPTAT25);
    ta = ta / params->KtPTAT + 25;
    
    return ta;
}

//------------------------------------------------------------------------------

int MLX90640_GetSubPageNumber(uint16_t *frameData)
{
    return frameData[833];    

}    

//------------------------------------------------------------------------------
void MLX90640_BadPixelsCorrection(uint16_t *pixels, float *to, int mode, paramsMLX90640 *params)
{   
    float ap[4];
    uint8_t pix;
    uint8_t line;
    uint8_t column;
    
    pix = 0;
    while(pixels[pix] != 0xFFFF)
    {
        line = pixels[pix]>>5;
        column = pixels[pix] - (line<<5);
        
        if(mode == 1)
        {        
            if(line == 0)
            {
                if(column == 0)
                {        
                    to[pixels[pix]] = to[33];                    
                }
                else if(column == 31)
                {
                    to[pixels[pix]] = to[62];                      
                }
                else
                {
                    to[pixels[pix]] = (to[pixels[pix]+31] + to[pixels[pix]+33])/2.0;                    
                }        
            }
            else if(line == 23)
            {
                if(column == 0)
                {
                    to[pixels[pix]] = to[705];                    
                }
                else if(column == 31)
                {
                    to[pixels[pix]] = to[734];                       
                }
                else
                {
                    to[pixels[pix]] = (to[pixels[pix]-33] + to[pixels[pix]-31])/2.0;                       
                }                       
            } 
            else if(column == 0)
            {
                to[pixels[pix]] = (to[pixels[pix]-31] + to[pixels[pix]+33])/2.0;                
            }
            else if(column == 31)
            {
                to[pixels[pix]] = (to[pixels[pix]-33] + to[pixels[pix]+31])/2.0;                
            } 
            else
            {
                ap[0] = to[pixels[pix]-33];
                ap[1] = to[pixels[pix]-31];
                ap[2] = to[pixels[pix]+31];
                ap[3] = to[pixels[pix]+33];
                to[pixels[pix]] = GetMedian(ap,4);
            }                   
        }
        else
        {        
            if(column == 0)
            {
                to[pixels[pix]] = to[pixels[pix]+1];            
            }
            else if(column == 1 || column == 30)
            {
                to[pixels[pix]] = (to[pixels[pix]-1]+to[pixels[pix]+1])/2.0;                
            } 
            else if(column == 31)
            {
                to[pixels[pix]] = to[pixels[pix]-1];
            } 
            else
            {
                if(IsPixelBad(pixels[pix]-2,params) == 0 && IsPixelBad(pixels[pix]+2,params) == 0)
                {
                    ap[0] = to[pixels[pix]+1] - to[pixels[pix]+2];
                    ap[1] = to[pixels[pix]-1] - to[pixels[pix]-2];
                    if(fabs(ap[0]) > fabs(ap[1]))
                    {
                        to[pixels[pix]] = to[pixels[pix]-1] + ap[1];                        
                    }
                    else
                    {
                        to[pixels[pix]] = to[pixels[pix]+1] + ap[0];                        
                    }
                }
                else
                {
                    to[pixels[pix]] = (to[pixels[pix]-1]+to[pixels[pix]+1])/2.0;                    
                }            
            }                      
        } 
        pix = pix + 1;    
    }    
}

//------------------------------------------------------------------------------

static void ExtractVDDParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    int8_t kVdd;
    int16_t vdd25;
    
    kVdd = MLX90640_MS_BYTE(eeData[51]);

    vdd25 = MLX90640_LS_BYTE(eeData[51]);
    vdd25 = ((vdd25 - 256) << 5) - 8192;
    
    mlx90640->kVdd = 32 * kVdd;
    mlx90640->vdd25 = vdd25; 
}

//------------------------------------------------------------------------------

static void ExtractPTATParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    float KvPTAT;
    float KtPTAT;
    int16_t vPTAT25;
    float alphaPTAT;
    
    KvPTAT = (eeData[50] & MLX90640_MSBITS_6_MASK) >> 10;
    if(KvPTAT > 31)
    {
        KvPTAT = KvPTAT - 64;
    }
    KvPTAT = KvPTAT/4096;
    
    KtPTAT = eeData[50] & MLX90640_LSBITS_10_MASK;
    if(KtPTAT > 511)
    {
        KtPTAT = KtPTAT - 1024;
    }
    KtPTAT = KtPTAT/8;
    
    vPTAT25 = eeData[49];
    
    alphaPTAT = (eeData[16] & MLX90640_NIBBLE4_MASK) / POW2(14) + 8.0f;
    
    mlx90640->KvPTAT = KvPTAT;
    mlx90640->KtPTAT = KtPTAT;    
    mlx90640->vPTAT25 = vPTAT25;
    mlx90640->alphaPTAT = alphaPTAT;   
}

//------------------------------------------------------------------------------

static void ExtractGainParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    mlx90640->gainEE = (int16_t)eeData[48];;    
}

//------------------------------------------------------------------------------

static void ExtractTgcParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    mlx90640->tgc = (int8_t)MLX90640_LS_BYTE(eeData[60]) / 32.0f;
}

//------------------------------------------------------------------------------

static void ExtractResolutionParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    uint8_t resolutionEE;
    resolutionEE = (eeData[56] & 0x3000) >> 12;    
    
    mlx90640->resolutionEE = resolutionEE;
}

//------------------------------------------------------------------------------

static void ExtractKsTaParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{   
    mlx90640->KsTa = (int8_t)MLX90640_MS_BYTE(eeData[60]) / 8192.0f;
}

//------------------------------------------------------------------------------

static void ExtractKsToParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    int32_t KsToScale;
    int8_t step;
    
    step = ((eeData[63] & 0x3000) >> 12) * 10;
    
    mlx90640->ct[0] = -40;
    mlx90640->ct[1] = 0;
    mlx90640->ct[2] = MLX90640_NIBBLE2(eeData[63]);
    mlx90640->ct[3] = MLX90640_NIBBLE3(eeData[63]);
    
    mlx90640->ct[2] = mlx90640->ct[2]*step;
    mlx90640->ct[3] = mlx90640->ct[2] + mlx90640->ct[3]*step;
    mlx90640->ct[4] = 400;
    
    KsToScale = MLX90640_NIBBLE1(eeData[63]) + 8;
    KsToScale = 1UL << KsToScale;
    
    mlx90640->ksTo[0] = (int8_t)MLX90640_LS_BYTE(eeData[61]) / (float)KsToScale;
    mlx90640->ksTo[1] = (int8_t)MLX90640_MS_BYTE(eeData[61]) / (float)KsToScale;
    mlx90640->ksTo[2] = (int8_t)MLX90640_LS_BYTE(eeData[62]) / (float)KsToScale;
    mlx90640->ksTo[3] = (int8_t)MLX90640_MS_BYTE(eeData[62]) / (float)KsToScale;
    mlx90640->ksTo[4] = -0.0002;
}

//------------------------------------------------------------------------------

static void ExtractAlphaParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    int accRow[24];
    int accColumn[32];
    int p = 0;
    int alphaRef;
    uint8_t alphaScale;
    uint8_t accRowScale;
    uint8_t accColumnScale;
    uint8_t accRemScale;
    float alphaTemp[768];
    float temp;
    

    accRemScale = MLX90640_NIBBLE1(eeData[32]);
    accColumnScale = MLX90640_NIBBLE2(eeData[32]);
    accRowScale = MLX90640_NIBBLE3(eeData[32]);
    alphaScale = MLX90640_NIBBLE4(eeData[32]) + 30;
    alphaRef = eeData[33];
    
    for(int i = 0; i < 6; i++)
    {
        p = i * 4;
        accRow[p + 0] = MLX90640_NIBBLE1(eeData[34 + i]);
        accRow[p + 1] = MLX90640_NIBBLE2(eeData[34 + i]);
        accRow[p + 2] = MLX90640_NIBBLE3(eeData[34 + i]);
        accRow[p + 3] = MLX90640_NIBBLE4(eeData[34 + i]);
    }
    
    for(int i = 0; i < MLX90640_LINE_NUM; i++)
    {
        if (accRow[i] > 7)
        {
            accRow[i] = accRow[i] - 16;
        }
    }
    
    for(int i = 0; i < 8; i++)
    {
        p = i * 4;
        accColumn[p + 0] = MLX90640_NIBBLE1(eeData[40 + i]);
        accColumn[p + 1] = MLX90640_NIBBLE2(eeData[40 + i]);
        accColumn[p + 2] = MLX90640_NIBBLE3(eeData[40 + i]);
        accColumn[p + 3] = MLX90640_NIBBLE4(eeData[40 + i]);
    }
    
    for(int i = 0; i < MLX90640_COLUMN_NUM; i++)
    {
        if (accColumn[i] > 7)
        {
            accColumn[i] = accColumn[i] - 16;
        }
    }

    for(int i = 0; i < MLX90640_LINE_NUM; i++)
    {
        for(int j = 0; j < MLX90640_COLUMN_NUM; j ++)
        {
            p = 32 * i +j;
            alphaTemp[p] = (eeData[64 + p] & 0x03F0) >> 4;
            if (alphaTemp[p] > 31)
            {
                alphaTemp[p] = alphaTemp[p] - 64;
            }
            alphaTemp[p] = alphaTemp[p]*(1 << accRemScale);
            alphaTemp[p] = (alphaRef + (accRow[i] << accRowScale) + (accColumn[j] << accColumnScale) + alphaTemp[p]);
            alphaTemp[p] = alphaTemp[p] / POW2(alphaScale);
            alphaTemp[p] = alphaTemp[p] - mlx90640->tgc * (mlx90640->cpAlpha[0] + mlx90640->cpAlpha[1])/2;
            alphaTemp[p] = SCALEALPHA/alphaTemp[p];
        }
    }
    
    temp = alphaTemp[0];
    for(int i = 1; i < MLX90640_PIXEL_NUM; i++)
    {
        if (alphaTemp[i] > temp)
        {
            temp = alphaTemp[i];
        }
    }
    
    alphaScale = 0;
    while(temp < 32767.4)
    {
        temp = temp*2;
        alphaScale = alphaScale + 1;
    } 
    
    for(int i = 0; i < MLX90640_PIXEL_NUM; i++)
    {
        temp = alphaTemp[i] * POW2(alphaScale);        
        mlx90640->alpha[i] = (temp + 0.5);        
        
    } 
    
    mlx90640->alphaScale = alphaScale;      
   
}

//------------------------------------------------------------------------------

static void ExtractOffsetParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    int occRow[24];
    int occColumn[32];
    int p = 0;
    int16_t offsetRef;
    uint8_t occRowScale;
    uint8_t occColumnScale;
    uint8_t occRemScale;
    

    occRemScale = MLX90640_NIBBLE1(eeData[16]);
    occColumnScale = MLX90640_NIBBLE2(eeData[16]);
    occRowScale = MLX90640_NIBBLE3(eeData[16]);
    offsetRef = (int16_t)eeData[17];
        
    for(int i = 0; i < 6; i++)
    {
        p = i * 4;
        occRow[p + 0] = MLX90640_NIBBLE1(eeData[18 + i]);
        occRow[p + 1] = MLX90640_NIBBLE2(eeData[18 + i]);
        occRow[p + 2] = MLX90640_NIBBLE3(eeData[18 + i]);
        occRow[p + 3] = MLX90640_NIBBLE4(eeData[18 + i]);
    }
    
    for(int i = 0; i < MLX90640_LINE_NUM; i++)
    {
        if (occRow[i] > 7)
        {
            occRow[i] = occRow[i] - 16;
        }
    }
    
    for(int i = 0; i < 8; i++)
    {
        p = i * 4;
        occColumn[p + 0] = MLX90640_NIBBLE1(eeData[24 + i]);
        occColumn[p + 1] = MLX90640_NIBBLE2(eeData[24 + i]);
        occColumn[p + 2] = MLX90640_NIBBLE3(eeData[24 + i]);
        occColumn[p + 3] = MLX90640_NIBBLE4(eeData[24 + i]);
    }
    
    for(int i = 0; i < MLX90640_COLUMN_NUM; i ++)
    {
        if (occColumn[i] > 7)
        {
            occColumn[i] = occColumn[i] - 16;
        }
    }

    for(int i = 0; i < MLX90640_LINE_NUM; i++)
    {
        for(int j = 0; j < MLX90640_COLUMN_NUM; j ++)
        {
            p = 32 * i +j;
            mlx90640->offset[p] = (eeData[64 + p] & MLX90640_MSBITS_6_MASK) >> 10;
            if (mlx90640->offset[p] > 31)
            {
                mlx90640->offset[p] = mlx90640->offset[p] - 64;
            }
            mlx90640->offset[p] = mlx90640->offset[p]*(1 << occRemScale);
            mlx90640->offset[p] = (offsetRef + (occRow[i] << occRowScale) + (occColumn[j] << occColumnScale) + mlx90640->offset[p]);
        }
    }
}

//------------------------------------------------------------------------------

static void ExtractKtaPixelParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    int p = 0;
    int8_t KtaRC[4];
    uint8_t ktaScale1;
    uint8_t ktaScale2;
    uint8_t split;
    float ktaTemp[768];
    float temp;
    
    KtaRC[0] = (int8_t)MLX90640_MS_BYTE(eeData[54]);;
    KtaRC[2] = (int8_t)MLX90640_LS_BYTE(eeData[54]);;
    KtaRC[1] = (int8_t)MLX90640_MS_BYTE(eeData[55]);;
    KtaRC[3] = (int8_t)MLX90640_LS_BYTE(eeData[55]);;
      
    ktaScale1 = MLX90640_NIBBLE2(eeData[56]) + 8;
    ktaScale2 = MLX90640_NIBBLE1(eeData[56]);

    for(int i = 0; i < MLX90640_LINE_NUM; i++)
    {
        for(int j = 0; j < MLX90640_COLUMN_NUM; j ++)
        {
            p = 32 * i +j;
            split = 2*(p/32 - (p/64)*2) + p%2;
            ktaTemp[p] = (eeData[64 + p] & 0x000E) >> 1;
            if (ktaTemp[p] > 3)
            {
                ktaTemp[p] = ktaTemp[p] - 8;
            }
            ktaTemp[p] = ktaTemp[p] * (1 << ktaScale2);
            ktaTemp[p] = KtaRC[split] + ktaTemp[p];
            ktaTemp[p] = ktaTemp[p] / POW2(ktaScale1);
            
        }
    }
    
    temp = fabs(ktaTemp[0]);
    for(int i = 1; i < MLX90640_PIXEL_NUM; i++)
    {
        if (fabs(ktaTemp[i]) > temp)
        {
            temp = fabs(ktaTemp[i]);
        }
    }
    
    ktaScale1 = 0;
    while(temp < 63.4)
    {
        temp = temp*2;
        ktaScale1 = ktaScale1 + 1;
    }    
     
    for(int i = 0; i < MLX90640_PIXEL_NUM; i++)
    {
        temp = ktaTemp[i] * POW2(ktaScale1);
        if (temp < 0)
        {
            mlx90640->kta[i] = (temp - 0.5);
        }
        else
        {
            mlx90640->kta[i] = (temp + 0.5);
        }        
        
    } 
    
    mlx90640->ktaScale = ktaScale1;           
}


//------------------------------------------------------------------------------

static void ExtractKvPixelParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    int p = 0;
    int8_t KvT[4];
    int8_t KvRoCo;
    int8_t KvRoCe;
    int8_t KvReCo;
    int8_t KvReCe;
    uint8_t kvScale;
    uint8_t split;
    float kvTemp[768];
    float temp;

    KvRoCo = MLX90640_NIBBLE4(eeData[52]);
    if (KvRoCo > 7)
    {
        KvRoCo = KvRoCo - 16;
    }
    KvT[0] = KvRoCo;
    
    KvReCo = MLX90640_NIBBLE3(eeData[52]);
    if (KvReCo > 7)
    {
        KvReCo = KvReCo - 16;
    }
    KvT[2] = KvReCo;
      
    KvRoCe = MLX90640_NIBBLE2(eeData[52]);
    if (KvRoCe > 7)
    {
        KvRoCe = KvRoCe - 16;
    }
    KvT[1] = KvRoCe;
      
    KvReCe = MLX90640_NIBBLE1(eeData[52]);
    if (KvReCe > 7)
    {
        KvReCe = KvReCe - 16;
    }
    KvT[3] = KvReCe;
  
    kvScale = MLX90640_NIBBLE3(eeData[56]);


    for(int i = 0; i < MLX90640_LINE_NUM; i++)
    {
        for(int j = 0; j < MLX90640_COLUMN_NUM; j ++)
        {
            p = 32 * i +j;
            split = 2*(p/32 - (p/64)*2) + p%2;
            kvTemp[p] = KvT[split];
            kvTemp[p] = kvTemp[p] / POW2(kvScale);
        }
    }
    
    temp = fabs(kvTemp[0]);
    for(int i = 1; i < MLX90640_PIXEL_NUM; i++)
    {
        if (fabs(kvTemp[i]) > temp)
        {
            temp = fabs(kvTemp[i]);
        }
    }
    
    kvScale = 0;
    while(temp < 63.4)
    {
        temp = temp*2;
        kvScale = kvScale + 1;
    }    
     
    for(int i = 0; i < MLX90640_PIXEL_NUM; i++)
    {
        temp = kvTemp[i] * POW2(kvScale);
        if (temp < 0)
        {
            mlx90640->kv[i] = (temp - 0.5);
        }
        else
        {
            mlx90640->kv[i] = (temp + 0.5);
        }        
        
    } 
    
    mlx90640->kvScale = kvScale;        
}

//------------------------------------------------------------------------------

static void ExtractCPParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    float alphaSP[2];
    int16_t offsetSP[2];
    float cpKv;
    float cpKta;
    uint8_t alphaScale;
    uint8_t ktaScale1;
    uint8_t kvScale;

    alphaScale = MLX90640_NIBBLE4(eeData[32]) + 27;
    
    offsetSP[0] = (eeData[58] & MLX90640_LSBITS_10_MASK);
    if (offsetSP[0] > 511)
    {
        offsetSP[0] = offsetSP[0] - 1024;
    }
    
    offsetSP[1] = (eeData[58] & MLX90640_MSBITS_6_MASK) >> 10;
    if (offsetSP[1] > 31)
    {
        offsetSP[1] = offsetSP[1] - 64;
    }
    offsetSP[1] = offsetSP[1] + offsetSP[0]; 
    
    alphaSP[0] = (eeData[57] & MLX90640_LSBITS_10_MASK);
    if (alphaSP[0] > 511)
    {
        alphaSP[0] = alphaSP[0] - 1024;
    }
    alphaSP[0] = alphaSP[0] /  POW2(alphaScale);
    
    alphaSP[1] = (eeData[57] & MLX90640_MSBITS_6_MASK) >> 10;
    if (alphaSP[1] > 31)
    {
        alphaSP[1] = alphaSP[1] - 64;
    }
    alphaSP[1] = (1 + alphaSP[1]/128) * alphaSP[0];
    
    cpKta = (int8_t)MLX90640_LS_BYTE(eeData[59]);
    
    ktaScale1 = MLX90640_NIBBLE2(eeData[56]) + 8;    
    mlx90640->cpKta = cpKta / POW2(ktaScale1);
    
    cpKv = (int8_t)MLX90640_MS_BYTE(eeData[59]);
    
    kvScale = MLX90640_NIBBLE3(eeData[56]);
    mlx90640->cpKv = cpKv / POW2(kvScale);
       
    mlx90640->cpAlpha[0] = alphaSP[0];
    mlx90640->cpAlpha[1] = alphaSP[1];
    mlx90640->cpOffset[0] = offsetSP[0];
    mlx90640->cpOffset[1] = offsetSP[1];  
}

//------------------------------------------------------------------------------

static void ExtractCILCParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    float ilChessC[3];
    uint8_t calibrationModeEE;
    
    calibrationModeEE = (eeData[10] & 0x0800) >> 4;
    calibrationModeEE = calibrationModeEE ^ 0x80;

    ilChessC[0] = (eeData[53] & 0x003F);
    if (ilChessC[0] > 31)
    {
        ilChessC[0] = ilChessC[0] - 64;
    }
    ilChessC[0] = ilChessC[0] / 16.0f;
    
    ilChessC[1] = (eeData[53] & 0x07C0) >> 6;
    if (ilChessC[1] > 15)
    {
        ilChessC[1] = ilChessC[1] - 32;
    }
    ilChessC[1] = ilChessC[1] / 2.0f;
    
    ilChessC[2] = (eeData[53] & 0xF800) >> 11;
    if (ilChessC[2] > 15)
    {
        ilChessC[2] = ilChessC[2] - 32;
    }
    ilChessC[2] = ilChessC[2] / 8.0f;
    
    mlx90640->calibrationModeEE = calibrationModeEE;
    mlx90640->ilChessC[0] = ilChessC[0];
    mlx90640->ilChessC[1] = ilChessC[1];
    mlx90640->ilChessC[2] = ilChessC[2];
}

//------------------------------------------------------------------------------

static int ExtractDeviatingPixels(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    uint16_t pixCnt = 0;
    uint16_t brokenPixCnt = 0;
    uint16_t outlierPixCnt = 0;
    int warn = 0;
    int i;
    
    for(pixCnt = 0; pixCnt<5; pixCnt++)
    {
        mlx90640->brokenPixels[pixCnt] = 0xFFFF;
        mlx90640->outlierPixels[pixCnt] = 0xFFFF;
    }
        
    pixCnt = 0;    
    while (pixCnt < MLX90640_PIXEL_NUM && brokenPixCnt < 5 && outlierPixCnt < 5)
    {
        if(eeData[pixCnt+64] == 0)
        {
            mlx90640->brokenPixels[brokenPixCnt] = pixCnt;
            brokenPixCnt = brokenPixCnt + 1;
        }    
        else if((eeData[pixCnt+64] & 0x0001) != 0)
        {
            mlx90640->outlierPixels[outlierPixCnt] = pixCnt;
            outlierPixCnt = outlierPixCnt + 1;
        }    
        
        pixCnt = pixCnt + 1;
        
    } 
    
    if(brokenPixCnt > 4)  
    {
        warn = -MLX90640_BROKEN_PIXELS_NUM_ERROR;
    }         
    else if(outlierPixCnt > 4)  
    {
        warn = -MLX90640_OUTLIER_PIXELS_NUM_ERROR;
    }
    else if((brokenPixCnt + outlierPixCnt) > 4)  
    {
        warn = -MLX90640_BAD_PIXELS_NUM_ERROR;
    } 
    else
    {
        for(pixCnt=0; pixCnt<brokenPixCnt; pixCnt++)
        {
            for(i=pixCnt+1; i<brokenPixCnt; i++)
            {
                warn = CheckAdjacentPixels(mlx90640->brokenPixels[pixCnt],mlx90640->brokenPixels[i]);
                if(warn != 0)
                {
                    return warn;
                }    
            }    
        }
        
        for(pixCnt=0; pixCnt<outlierPixCnt; pixCnt++)
        {
            for(i=pixCnt+1; i<outlierPixCnt; i++)
            {
                warn = CheckAdjacentPixels(mlx90640->outlierPixels[pixCnt],mlx90640->outlierPixels[i]);
                if(warn != 0)
                {
                    return warn;
                }    
            }    
        } 
        
        for(pixCnt=0; pixCnt<brokenPixCnt; pixCnt++)
        {
            for(i=0; i<outlierPixCnt; i++)
            {
                warn = CheckAdjacentPixels(mlx90640->brokenPixels[pixCnt],mlx90640->outlierPixels[i]);
                if(warn != 0)
                {
                    return warn;
                }    
            }    
        }    
        
    }    
    
    
    return warn;
       
}

//------------------------------------------------------------------------------

 static int CheckAdjacentPixels(uint16_t pix1, uint16_t pix2)
 {
     int pixPosDif;
     
     pixPosDif = pix1 - pix2;
     if(pixPosDif > -34 && pixPosDif < -30)
     {
         return -MLX90640_ADJACENT_BAD_PIXELS_ERROR;
     } 
     if(pixPosDif > -2 && pixPosDif < 2)
     {
         return -MLX90640_ADJACENT_BAD_PIXELS_ERROR;
     } 
     if(pixPosDif > 30 && pixPosDif < 34)
     {
         return -MLX90640_ADJACENT_BAD_PIXELS_ERROR;
     }
     
     return 0;    
 }
 
//------------------------------------------------------------------------------
 
static float GetMedian(float *values, int n)
 {
    float temp;
    
    for(int i=0; i<n-1; i++)
    {
        for(int j=i+1; j<n; j++)
        {
            if(values[j] < values[i]) 
            {                
                temp = values[i];
                values[i] = values[j];
                values[j] = temp;
            }
        }
    }
    
    if(n%2==0) 
    {
        return ((values[n/2] + values[n/2 - 1]) / 2.0);
        
    } 
    else 
    {
        return values[n/2];
    }
    
 }           

//------------------------------------------------------------------------------

static int IsPixelBad(uint16_t pixel,paramsMLX90640 *params)
{
    for(int i=0; i<5; i++)
    {
        if(pixel == params->outlierPixels[i] || pixel == params->brokenPixels[i])
        {
            return 1;
        }    
    }   
    
    return 0;     
}     

//------------------------------------------------------------------------------
