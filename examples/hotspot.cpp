#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <math.h>
#include "headers/MLX90640_API.h"
#include "lib/fb.h"

#define MLX_I2C_ADDR 0x33

#define IMAGE_SCALE 5

// Valid frame rates are 1, 2, 4, 8, 16, 32 and 64
// The i2c baudrate is set to 1mhz to support these
#define FPS 8
#define FRAME_TIME_MICROS (1000000/FPS)

// Despite the framerate being ostensibly FPS hz
// The frame is often not ready in time
// This offset is added to the FRAME_TIME_MICROS
// to account for this.
#define OFFSET_MICROS 850

uint8_t font[] = {
    0b01111110,
    0b10000001,
    0b10000001,
    0b10000001,
    0b01111110,

    0b00000000,
    0b00000001,
    0b11111111,
    0b10000001,
    0b00000000,

    0b01100001,
    0b10010001,
    0b10010001,
    0b10010001,
    0b10001111,

    0b01101110,
    0b10010001,
    0b10010001,
    0b10010001,
    0b10010001,

    0b11111111,
    0b00010000,
    0b00010000,
    0b00010000,
    0b11100000,

    0b10001111,
    0b10010001,
    0b10010001,
    0b10010001,
    0b01100001,

    0b10001110,
    0b10010001,
    0b10010001,
    0b10010001,
    0b01111110,

    0b11111111,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,

    0b01101110,
    0b10010001,
    0b10010001,
    0b10010001,
    0b01101110,

    0b11111111,
    0b10010000,
    0b10010000,
    0b10010000,
    0b01100000
};

void put_pixel_scaled(int x, int y, int r, int g, int b){
    x *= IMAGE_SCALE;
    y *= IMAGE_SCALE;
    for (int px = 0; px < IMAGE_SCALE; px++){
        for (int py = 0; py < IMAGE_SCALE; py++){
            fb_put_pixel(x + px, y + py, r, g, b);
        }
    }
}

void put_digit(int x, int y, int number) {
    for (int n_x = 0; n_x < 5; n_x++) {
        int col = font[(number * 5) + (4-n_x)];
        for (int n_y = 0; n_y < 8; n_y++) {
            int pixel = (col & 0b10000000) ? 255 : 0;
            col <<= 1;
            put_pixel_scaled(x + n_x, y + n_y, pixel, pixel, pixel);
        }
    }   
}

void put_number(int x, int y, float number) {
    float div = 0.01;
    unsigned int digits = 1;
    unsigned int o_x = 0;
    if (number > 999.99){
        number = 999.99;
    }
    while (div <= number / 10) {
        digits++;
        div *= 10;
    }
    if (number < 100) {
        put_digit(x + o_x, y, 0);
        o_x += 6;
    }
    while (digits > 0) {
        put_digit(x + o_x, y, number / div);
        number = fmod(number, div); 
        div /= 10;
        if (digits == 3){
            put_pixel_scaled(x + o_x + 6, y + 7, 255, 255, 255);
            o_x += 2;
        }
        digits--;        
        o_x += 6;
    }
}

void put_pixel_false_colour(int x, int y, double v) {
    // Heatmap code borrowed from: http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
    const int NUM_COLORS = 7;
    static float color[NUM_COLORS][3] = { {0,0,0}, {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0}, {1,0,1}, {1,1,1} };
    int idx1, idx2;
    float fractBetween = 0;
    float vmin = 5.0;
    float vmax = 50.0;
    float vrange = vmax-vmin;
    v -= vmin;
    v /= vrange;
    if(v <= 0) {idx1=idx2=0;}
    else if(v >= 1) {idx1=idx2=NUM_COLORS-1;}
    else
    {
        v *= (NUM_COLORS-1);
        idx1 = floor(v);
        idx2 = idx1+1;
        fractBetween = v - float(idx1);
    }

    int ir, ig, ib;


    ir = (int)((((color[idx2][0] - color[idx1][0]) * fractBetween) + color[idx1][0]) * 255.0);
    ig = (int)((((color[idx2][1] - color[idx1][1]) * fractBetween) + color[idx1][1]) * 255.0);
    ib = (int)((((color[idx2][2] - color[idx1][2]) * fractBetween) + color[idx1][2]) * 255.0);

    put_pixel_scaled(x, y, ir, ig, ib);
    /*for(int px = 0; px < IMAGE_SCALE; px++){
        for(int py = 0; py < IMAGE_SCALE; py++){
            fb_put_pixel(x + px, y + py, ir, ig, ib);
        }
    }*/
}

int main(){
    static uint16_t eeMLX90640[832];
    float emissivity = 1;
    uint16_t frame[834];
    static float image[768];
    static float mlx90640To[768];
    float eTa;
    static uint16_t data[768*sizeof(float)];

    auto frame_time = std::chrono::microseconds(FRAME_TIME_MICROS + OFFSET_MICROS);

    MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0);
    MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0);
    switch(FPS){
        case 1:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b001);
            break;
        case 2:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b010);
            break;
        case 4:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b011);
            break;
        case 8:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b100);
            break;
        case 16:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b101);
            break;
        case 32:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b110);
            break;
        case 64:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b111);
            break;
        default:
            printf("Unsupported framerate: %d", FPS);
            return 1;
    }
    MLX90640_SetChessMode(MLX_I2C_ADDR);

    paramsMLX90640 mlx90640;
    MLX90640_DumpEE(MLX_I2C_ADDR, eeMLX90640);
    MLX90640_ExtractParameters(eeMLX90640, &mlx90640);

    fb_init();

    while (1){
        float hotspot = 0;
        int hotspot_x = 0;
        int hotspot_y = 0;
        auto start = std::chrono::system_clock::now();
        MLX90640_GetFrameData(MLX_I2C_ADDR, frame);
        MLX90640_InterpolateOutliers(frame, eeMLX90640);

        eTa = MLX90640_GetTa(frame, &mlx90640);
        MLX90640_CalculateTo(frame, &mlx90640, emissivity, eTa, mlx90640To);

        for(int y = 0; y < 24; y++){
            for(int x = 0; x < 32; x++){
                float val = mlx90640To[32 * (23-y) + x];
                if (val > hotspot){
                    hotspot = val;
                    hotspot_x = y;
                    hotspot_y = x;
                }
                put_pixel_false_colour(y, x, val);
            }
        }

        if(hotspot_x - 1 >= 0){
            put_pixel_scaled(hotspot_x - 1, hotspot_y, 255, 255, 255);
        }
        if(hotspot_x + 1 < 24){
            put_pixel_scaled(hotspot_x + 1, hotspot_y, 255, 255, 255);
        }
        if(hotspot_y - 1 >= 0){
            put_pixel_scaled(hotspot_x, hotspot_y - 1, 255, 255, 255);
        }
        if(hotspot_y + 1 < 32){
            put_pixel_scaled(hotspot_x, hotspot_y + 1, 255, 255, 255);
        }

        put_number(0, 33, hotspot);

        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::this_thread::sleep_for(std::chrono::microseconds(frame_time - elapsed));
    }

    fb_cleanup();
    return 0;
}
