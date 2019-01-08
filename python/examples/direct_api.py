import sys
from pathlib import Path
import time

import numpy as np

build_path = Path(__file__).resolve().parents[1] / "build" /  "lib.linux-armv7l-3.7"
sys.path.append(str(build_path))

from MLX90640 import API, ffi, temperature_data_to_ndarray

MLX_I2C_ADDR = 0x33

hertz = 8

hertzSettings = {
    0.5: 0,
    1: 1,
    2: 2,
    4: 3,
    8: 4,
    16: 5,
    32: 6,
    64: 7
}

# settings
API.SetRefreshRate(MLX_I2C_ADDR, hertzSettings[hertz])
API.SetChessMode(MLX_I2C_ADDR)

# POR
time.sleep(.08) # wait 80ms
time.sleep(2/hertz) # delay det by refresh rate

# Extract calibration data from EEPROM and store in RAM
eeprom_data = ffi.new("uint16_t[832]")
params = ffi.new("paramsMLX90640*")
API.DumpEE(MLX_I2C_ADDR, eeprom_data)
API.ExtractParameters(eeprom_data, params)
print(params.KsTa)
print(params.kVdd)
for i in range(5):
    print(params.brokenPixels[i])
for i in range(5):
    print(params.outlierPixels[i])

# TODO: if absolute - wait 4 mins

TA_SHIFT = 8 # the default shift for a MLX90640 device in open air
emissivity = 0.95

frame_buffer = ffi.new("uint16_t[834]")
image_buffer = ffi.new("float[768]")

print("Calc Hertz should be close to chosen value (%s)" % hertz)
last = time.monotonic()
while True:
    API.GetFrameData(MLX_I2C_ADDR, frame_buffer);
    now = time.monotonic()
    diff = now - last
    print("Calc Hz: %s" % (1/diff))
    last = now

    # reflected temperature based on the sensor
    # ambient temperature
    tr = API.GetTa(frame_buffer, params) - TA_SHIFT

    # The object temperatures for all 768 pixels in a
    # frame are stored in the mlx90640To array
    API.CalculateTo(frame_buffer, params, emissivity, tr, image_buffer);
    print("Subpage no: %s" % API.GetSubPageNumber(frame_buffer))

    print(temperature_data_to_ndarray(image_buffer))



