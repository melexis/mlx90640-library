#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <math.h>
#include "MLX90640/MLX90640_API.h"
#include "MLX90640/MLX90640_I2C_Driver.h"

#define MLX_I2C_ADDR 0x33

// Despite the framerate being ostensibly FPS hz
// The frame is often not ready in time
// This offset is added to the frame time
// to account for this.
#define OFFSET_MICROS 850

int baudrate = 400000;
paramsMLX90640 mlx90640;
static uint16_t eeMLX90640[832];
float emissivity = 1;
uint16_t frame[834];
// static float image[768];
static float mlx90640To[768];
float eTa;
// static uint16_t data[768*sizeof(float)];

//extern "C" 
int setup(int fps){
	MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0);
	MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0);

	//int t = (1000000 / fps) + OFFSET_MICROS;

	switch(fps){
		case 1:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b001);
			baudrate = 400000;
			break;
		case 2:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b010);
			baudrate = 400000;
			break;
		case 4:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b011);
			baudrate = 400000;
			break;
		case 8:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b100);
			baudrate = 400000;
			break;
		case 16:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b101);
			baudrate = 1000000;
			break;
		case 32:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b110);
			baudrate = 1000000;
			break;
		case 64:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b111);
			baudrate = 1000000;
			break;
		default:
#ifdef DEBUG
			printf("Unsupported framerate: %d", fps);
#endif
			return 1;
	}
	MLX90640_SetChessMode(MLX_I2C_ADDR);
	MLX90640_DumpEE(MLX_I2C_ADDR, eeMLX90640);
	MLX90640_ExtractParameters(eeMLX90640, &mlx90640);

	return 0;
}

//extern "C" 
void cleanup(void){
	//nothing...
}

//extern "C" 
float * get_frame(void){
	int retries = 6;
	int subpage;
	bool subpages[2] = {0,0};

	retries=10;

	while (retries-- && (!subpages[0] || !subpages[1])){
#ifdef DEBUG
		printf("Retries: %d \n", retries);
#endif
		//auto start = std::chrono::system_clock::now();

		MLX90640_GetFrameData(MLX_I2C_ADDR, frame);
#ifdef DEBUG
		printf("Got data for page %d\n", MLX90640_GetSubPageNumber(frame));
#endif
		subpage = MLX90640_GetSubPageNumber(frame);

		subpages[subpage] = 1;

#ifdef DEBUG
		printf("Converting data for page %d\n", subpage);
#endif

		eTa = MLX90640_GetTa(frame, &mlx90640);
		MLX90640_CalculateTo(frame, &mlx90640, emissivity, eTa, mlx90640To);

		MLX90640_BadPixelsCorrection((&mlx90640)->brokenPixels, mlx90640To, 1, &mlx90640);
		MLX90640_BadPixelsCorrection((&mlx90640)->outlierPixels, mlx90640To, 1, &mlx90640);
	}
#ifdef DEBUG
	printf("Finishing\n");
#endif

	return mlx90640To;
}
