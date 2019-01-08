# mlx90640-library

MLX90640 library functions

# Python

This fork also incorporates code from pimoroni and pjaos. 

To run the python module as pi user (non-root). Use:

sudo usermod -a -G i2c pi

To build (requires numpy/cffi):

python setup.py build

Run examples in examples folder. Need pygame + x server to run gui.
