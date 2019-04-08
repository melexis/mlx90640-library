#!/usr/bin/env python3
import time
from datetime import datetime
import subprocess
import sys
from PIL import Image, ImageSequence
import os

MAX_FRAMES = 50           # Large sizes get big quick!
OUTPUT_SIZE = (240, 320)  # Multiple of (24, 32)
FPS = 16                  # Should match the FPS value in examples/rawrgb.cpp
RAW_RGB_PATH = "../examples/rawrgb"

frames = []

if not os.path.isfile(RAW_RGB_PATH):
    raise RuntimeError("{} doesn't exist, did you forget to run \"make\"?".format(RAW_RGB_PATH))

print("""rgb-to-gif.py - output a gif using ./rawrgb command.

Use ./rawrgb to grab frames from the MLX90640
and render them into a gif.

You must have built the "rawrgb" executable first with "make"

Press Ctrl+C to save & exit!

""")

try:
    with subprocess.Popen(["sudo", "../rawrgb"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as camera:
        while True:
            # Despite the docs, we use read() here since we want to poll
            # the process for chunks of 2304 bytes, each of which is a frame
            frame = camera.stdout.read(2304)
            size = len(frame)
            print("Got {} bytes of data!".format(size))

            # Convert the raw frame bytes into a PIL image and resize
            image = Image.frombytes('RGB', (24, 32), frame)
            image = image.resize(OUTPUT_SIZE, Image.NEAREST)

            frames.append(image)
            print("Frames: {}".format(len(frames)))
            if len(frames) == MAX_FRAMES:
                break

            time.sleep(1.0 / FPS)

except KeyboardInterrupt:
    pass
finally:
    if len(frames) > 1:
        filename = 'mlx90640-{}.gif'.format(
            datetime.now().strftime("%Y-%m-%d-%H-%M-%S"))
        print("Saving {} with {} frames.".format(filename, len(frames)))
        frames[0].save(
            filename,
            save_all=True,
            append_images=frames[1:],
            duration=1000 // FPS,
            loop=0)    
