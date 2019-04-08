# MLX90640 Python Bindings

## Subprocess

We have included a Python example - `rgb-to-gif.py` - which uses the `rawrgb` C++ example along with PIL to produce a 50 frame .gif animation from the MLX90640 sensor.

This example uses Python's subprocess module to offload the task of dealing with the sensor, and it works pretty well!

To get it running, you'll need to build all the C++ examples. If this is your current working directory you can do that with `make -C .. examples/rawrgb`, otherwise type `make examples/rawrgb` in the root of this repository.

Then run `./rgb-to-gif.py`

This script will run for 50 frames by default, and save the result to a datestamped gif file at 240x320 resolution.

## Experimental Library

An experimental python library can be found in the library/ folder along with a simple `test.py` script.
