import sys
import time

import numpy as np
import pygame
from PIL import Image
import matplotlib as mpl
from matplotlib import cm

from MLX90640 import API, ffi, temperature_data_to_ndarray, hertz_to_refresh_rate

def td_to_image(f, cmap):
    norm = mpl.colors.Normalize(vmin=f.min(),vmax=f.max())
    img = Image.fromarray(np.uint8(cmap(norm(f))*255))
    img = img.convert("RGB").resize((320,240), Image.BICUBIC)
    return img

def increment_refresh_rate():
    rr = API.GetRefreshRate(MLX_I2C_ADDR)
    new_rr = (rr+1) % 8
    print(f"Set new refresh rate to {new_rr}")
    API.SetRefreshRate(MLX_I2C_ADDR, new_rr)

def show_text(display, text, pos, font, action=None):
    surf = font.render(text, False, (255,255,255))
    if action and pygame.mouse.get_pressed()[0] and surf.get_rect().move(pos).collidepoint(pygame.mouse.get_pos()):
        action()
    display.blit(surf, pos)

def main():
    # setup colour map
    cmap = cm.get_cmap('Spectral_r')

    # set up display
    pygame.init()
    pygame.font.init()
    display = pygame.display.set_mode((320, 240))
    pygame.display.set_caption('Thermal Cam')
    pygame.mouse.set_visible(True)
    font = pygame.font.SysFont('freemono', 10)

    # mlx90640 settings
    MLX_I2C_ADDR = 0x33
    hertz_default = 8
    API.SetRefreshRate(MLX_I2C_ADDR, hertz_to_refresh_rate[hertz_default])
    API.SetChessMode(MLX_I2C_ADDR)

    # Extract calibration data from EEPROM and store in RAM
    eeprom_data = ffi.new("uint16_t[832]")
    params = ffi.new("paramsMLX90640*")
    API.DumpEE(MLX_I2C_ADDR, eeprom_data)
    API.ExtractParameters(eeprom_data, params)

    TA_SHIFT = 8 # the default shift for a MLX90640 device in open air
    emissivity = 0.95

    frame_buffer = ffi.new("uint16_t[834]")
    image_buffer = ffi.new("float[768]")

    last = time.monotonic()
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                quit()

        API.GetFrameData(MLX_I2C_ADDR, frame_buffer);
        now = time.monotonic()
        diff = now - last
        last = now

        # reflected temperature based on the sensor
        # ambient temperature
        tr = API.GetTa(frame_buffer, params) - TA_SHIFT

        # The object temperatures for all 768 pixels in a
        # frame are stored in the mlx90640To array
        API.CalculateTo(frame_buffer, params, emissivity, tr, image_buffer);

        ta_np = temperature_data_to_ndarray(image_buffer)
        ta_img = td_to_image(ta_np, cmap)

        pyg_img = pygame.image.fromstring(ta_img.tobytes(), ta_img.size, ta_img.mode)
        display.blit(pyg_img, (0,0))

        show_text(display, "Calc Hz: %03.1f" % (1/diff), (0, 10+2), font, action=increment_refresh_rate)
        show_text(display, "Temp Ref: %05.1f" % tr, (0, 10+2+10+2), font)

        pygame.display.update()

if __name__ == "__main__":
    main()

