#include <iostream>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include "mlx90640_driver.h"



int main(){
    testpic pic;
    MLX90640 mlx90640;
    mlx90640.registerCallback(&pic);
    mlx90640.MLX90640_I2CInit();
    getchar();
    mlx90640.stop();

    return 0;
}
