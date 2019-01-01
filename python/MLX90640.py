import cffi
from pathlib import Path

import numpy as np

lib_path = Path(__file__).parent / "_MLX90640.cpython-37m-arm-linux-gnueabihf.so"

ffi = cffi.FFI()
ffi.cdef("int setup(int fps);")
ffi.cdef("float *  get_frame(void);")
C = ffi.dlopen(str(lib_path))

def setup(fps):
    C.setup(fps)

def get_frame():
    a = np.frombuffer(ffi.buffer(C.get_frame(), np.dtype(np.float32).itemsize*768), dtype=np.float32)
    return a.reshape((24, 32))

if __name__ == "__main__":
    setup(16)
    f = get_frame()
    print(f.min())
    print(f.max())
    print(f)


