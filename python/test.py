import sys
import colorsys
from PIL import Image
sys.path.insert(0, "./build/lib.linux-armv7l-2.7")

import MLX90640 as x

img = Image.new( 'RGB', (24,32), "black")

def temp_to_col(val):
    hue = (180 - (val * 6)) / 360.0
    return tuple([int(c*255) for c in colorsys.hsv_to_rgb(hue % 1, 1.0, 1.0)])

# def temp_to_col(val):
#     return tuple([int(c*255) for c in colorsys.hsv_to_rgb(1.0, 0.0, val/50.0)])


x.setup(16)
f = x.get_frame()
x.cleanup()

v_min = min(f)
v_max = max(f)

print(min(f))
print(max(f))

print(",".join(["{:05b}".format(x) for x in range(24)]))
for x in range(24):
    row = []
    for y in range(32):
        val = f[32 * (23-x) + y]
        row.append(val)
        img.putpixel((x, y), temp_to_col(val))
    print(",".join(["{:05.2f}".format(v) for v in row]))

img = img.resize((480,640), Image.BICUBIC)

img.save("test6.png")

