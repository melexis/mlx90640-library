import cffi
from pathlib import Path

import numpy as np

lib_path = Path(__file__).parent / "_MLX90640.cpython-37m-arm-linux-gnueabihf.so"

ffi = cffi.FFI()
ffi.cdef("""
typedef struct {
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
float ksTo[4];
int16_t ct[4];
float alpha[768];
int16_t offset[768];
float kta[768];
float kv[768];
float cpAlpha[2];
int16_t cpOffset[2];
float ilChessC[3];
uint16_t brokenPixels[5];
uint16_t outlierPixels[5];
} paramsMLX90640;

int DumpEE(uint8_t slaveAddr, uint16_t *eeData);
int GetFrameData(uint8_t slaveAddr, uint16_t *frameData);
int ExtractParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
float GetVdd(uint16_t *frameData, const paramsMLX90640 *params);
float GetTa(uint16_t *frameData, const paramsMLX90640 *params);
void GetImage(uint16_t *frameData, const paramsMLX90640 *params, float *result);
void CalculateTo(uint16_t *frameData, const paramsMLX90640 *params, float emissivity, float tr, float *result);
int SetResolution(uint8_t slaveAddr, uint8_t resolution);
int GetCurResolution(uint8_t slaveAddr);
int SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate);
int GetRefreshRate(uint8_t slaveAddr);
int GetSubPageNumber(uint16_t *frameData);
int GetCurMode(uint8_t slaveAddr);
int SetInterleavedMode(uint8_t slaveAddr);
int SetChessMode(uint8_t slaveAddr);
void BadPixelsCorrection(uint16_t *pixels, float *to, int mode, paramsMLX90640 *params);

""")

API = ffi.dlopen(str(lib_path))

def temperature_data_to_ndarray(frame):
    """Converts c buffer storing temp data to numpy array of shape 24, 32"""
    a = np.frombuffer(ffi.buffer(frame, np.dtype(np.float32).itemsize*768), dtype=np.float32)
    return a.reshape((24, 32))

_hertz_refreshrate_pairs = [ 
    (0.5, 0), (1, 1), (2, 2), (4, 3),
    (8, 4), (16, 5), (32, 6), (64, 7)]

hertz_to_refresh_rate = {h: rr for h, rr in _hertz_refreshrate_pairs}
refresh_rate_to_hertz = {rr: h for h, rr in _hertz_refreshrate_pairs}

