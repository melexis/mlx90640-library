# mlx90640-library
MLX90640 library functions

## Raspberry Pi Users

** EXPERIMENTAL **

This port uses the bcm2835 library, install like so:

```text
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.55.tar.gz
tar xvfz bcm2835-1.55.tar.gz
cd bcm2835-1.55
./configure
make
sudo make install
```

Then just "make" and "sudo ./test" or one of the other examples listed below:

# fbuf

```
sudo ./fbuf
```

This example uses direct-to-framebuffer rendering and black-blue-green-yellow-red-purple-white false colouring.

If you gave issues with the output image, set "`IMAGE_SCALE`" to a smaller number.

# interp

```
sudo ./interp
```

This example uses direct-to-framebuffer rendering and black-blue-green-yellow-red-purple-white false colouring.

It also has 2x bicubic resize filter.

If you have issues with the output image, set "`IMAGE_SCALE`" to a smaller number.

# test


```
sudo ./test
```

This example draws out to the console using ANSI colours and the full block char.

To see the actual temperature values, change "`FMT_STRING`" from the block char to the float format.

# step

```
sudo ./step
```

Attempt to run in step by step mode (experimental)

