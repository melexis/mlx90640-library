/* 
 * Borrowed from https://github.com/adafruit/Adafruit_AMG88xx/blob/master/examples/thermal_cam_interpolate/interpolation.cpp
 *
 * The text below is included in accordance with the MIT license conditions in the README of the above repository:
 *
 * """
 * This is a library for the Adafruit AMG88xx based thermal cameras:
 *
 * https://www.adafruit.com/products/3538
 * https://www.adafruit.com/product/3622
 * Check out the links above for our tutorials and wiring diagrams. This chip uses I2C to communicate
 *
 * Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!
 *
 * Written by Dean Miller for Adafruit Industries. MIT license, all text above must be included in any redistribution
 * """
 */

#include <stdint.h>

#ifndef interpolate_h
#define interpolate_h
#ifdef __cplusplus 
extern "C" {
#endif
float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f);
void get_adjacents_1d(float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void get_adjacents_2d(float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
float cubicInterpolate(float p[], float x);
float bicubicInterpolate(float p[], float x, float y);
void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols, 
                       float *dest, uint8_t dest_rows, uint8_t dest_cols);
#ifdef __cplusplus
}
#endif
#endif
