import cffi
from pathlib import Path

lib_path = Path(__file__).parent / "_MLX90640.cpython-37m-arm-linux-gnueabihf.so"

ffi = cffi.FFI()
ffi.cdef("int setup(int fps);")
ffi.cdef("float *  get_frame(void);")
C = ffi.dlopen(str(lib_path))

def setup(fps):
    C.setup(fps)

def get_frame():
    f = C.get_frame()
    f = [f[i] for i in range(768)]
    return f



if __name__ == "__main__":
    # this is original test code from pimoroni
    import sys
    import colorsys
    from PIL import Image

    img = Image.new( 'RGB', (24,32), "black")

    def temp_to_col(val):
        hue = (180 - (val * 6)) / 360.0
        return tuple([int(c*255) for c in colorsys.hsv_to_rgb(hue, 1.0, 1.0)])

    #def temp_to_col(val):
    #    return tuple([int(c*255) for c in colorsys.hsv_to_rgb(1.0, 0.0, val/50.0)])


    setup(16)
    f = get_frame()
    print(f)

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

