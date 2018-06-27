# mlx90640-library
MLX90640 library functions

## Raspberry Pi Users

** EXPERIMENTAL **

This port uses the bcm2835 library, install like so:

```text
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.55.tar.gz
tar xvfz bcm2835-1.55.tar.gz
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

# test


```
sudo ./test
```

This example draws out to the console using ANSI colours and the full block char.

# step

```
sudo ./step
```

Attempt to run in step by step mode (experimental)

