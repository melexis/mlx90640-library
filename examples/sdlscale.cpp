#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <math.h>
#include "headers/MLX90640_API.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>

#define MLX_I2C_ADDR 0x33

#define SENSOR_W 24
#define SENSOR_H 32

// Valid frame rates are 1, 2, 4, 8, 16, 32 and 64
// The i2c baudrate is set to 1mhz to support these
#define FPS 8
#define FRAME_TIME_MICROS (1000000/FPS)

// Despite the framerate being ostensibly FPS hz
// The frame is often not ready in time
// This offset is added to the FRAME_TIME_MICROS
// to account for this.
#define OFFSET_MICROS 850

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
SDL_Texture *texture_r = NULL;
SDL_Event event;

SDL_Rect rect_preserve_aspect;
SDL_Rect rect_fullscreen;

uint32_t pixels[SENSOR_W * SENSOR_H];

bool running = true;
bool preserve_aspect = true;

int rotation = 0;


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

    int offset = (y * SENSOR_W + x);

    pixels[offset] = (ib << 16) | (ig << 8) | (ir << 0);
}

int main(void) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init() Failed: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("MLX90640", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
    if(window == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow() Failed: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateRenderer() Failed: %s\n", SDL_GetError());
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SENSOR_W, SENSOR_H);
    if(texture == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateTexture() Failed: %s\n", SDL_GetError());
    }

    texture_r = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, SENSOR_H, SENSOR_H);
    if(texture_r == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateTexture() Failed: %s\n", SDL_GetError());
    }


    int display_width, display_height;

    SDL_GetRendererOutputSize(renderer, &display_width, &display_height);

    int aspect_scale = display_height / SENSOR_H;

    int output_width, output_height;

    output_width = SENSOR_W * aspect_scale;
    output_height = SENSOR_H * aspect_scale;

    int offset_left, offset_top;

    offset_left = (display_width - output_width) / 2;
    offset_top = (display_height - output_height) / 2;

    rect_preserve_aspect = (SDL_Rect){.x = offset_left, .y = offset_top, .w = output_width, .h = output_height};
    rect_fullscreen = (SDL_Rect){.x = 0, .y = 0, .w = display_width, .h = display_height};

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

    while(running){
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                running = false;
                break;
            }
            if(event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym){
                    case SDLK_SPACE:
                        preserve_aspect = !preserve_aspect;
                        break;
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_r:
                        rotation += 90;
                        if(rotation == 360){
                            rotation = 0;
                        }
                        break;
                }
            }
        }

        auto start = std::chrono::system_clock::now();
        MLX90640_GetFrameData(MLX_I2C_ADDR, frame);

        eTa = MLX90640_GetTa(frame, &mlx90640);
        MLX90640_CalculateTo(frame, &mlx90640, emissivity, eTa, mlx90640To);

        MLX90640_BadPixelsCorrection((&mlx90640)->brokenPixels, mlx90640To, 1, &mlx90640);
        MLX90640_BadPixelsCorrection((&mlx90640)->outlierPixels, mlx90640To, 1, &mlx90640);

        for(int y = 0; y < SENSOR_W; y++){
            for(int x = 0; x < SENSOR_H; x++){
                float val = mlx90640To[SENSOR_H * (SENSOR_W-1-y) + x];
                put_pixel_false_colour(y, x, val);
            }
        }

        SDL_UpdateTexture(texture, NULL, (uint8_t *)pixels, SENSOR_W * sizeof(uint32_t));

        SDL_SetRenderTarget(renderer, texture_r);
        SDL_RenderCopyEx(renderer, texture, NULL, NULL, rotation, NULL, SDL_FLIP_NONE);
        SDL_SetRenderTarget(renderer, NULL);

        if(preserve_aspect){
            SDL_RenderCopy(renderer, texture_r, NULL, &rect_preserve_aspect);
        }
        else
        {
            SDL_RenderCopy(renderer, texture_r, NULL, &rect_fullscreen);
        }

        SDL_RenderPresent(renderer);

        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::this_thread::sleep_for(std::chrono::microseconds(frame_time - elapsed));
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
