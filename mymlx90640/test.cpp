#include <iostream>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include "mlx90640_driver.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_NONE    "\x1b[30m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//#define FMT_STRING "%+06.2f "
#define FMT_STRING "\u2588\u2588"

#define MLX_I2C_ADDR 0x33

bool readd;
testpic pic;

int main(){

    MLX90640 mlx90640;
    mlx90640.registerCallback(&pic);
    mlx90640.MLX90640_I2CInit();

    return 0;
}