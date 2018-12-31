#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <math.h>
#include "../headers/MLX90640_API.h"
#include "../headers/MLX90640_I2C_Driver.h"

#define MLX_I2C_ADDR 0x33

// Despite the framerate being ostensibly FPS hz
// The frame is often not ready in time
// This offset is added to the frame time
// to account for this.
#define OFFSET_MICROS 850

paramsMLX90640 mlx90640;
static uint16_t eeMLX90640[832];
float emissivity = 1;
uint16_t frame[834];
static float mlx90640To[768];
float eTa;
auto frame_time = std::chrono::microseconds(0);

extern "C" int setup(int fps){
	int t = (1000000 / fps) + OFFSET_MICROS;
	frame_time = std::chrono::microseconds(t);

	switch(fps){
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
			printf("Unsupported framerate: %d", fps);
			return 1;
	}
	MLX90640_SetChessMode(MLX_I2C_ADDR);
	MLX90640_DumpEE(MLX_I2C_ADDR, eeMLX90640);
	MLX90640_ExtractParameters(eeMLX90640, &mlx90640);

	return 0;
}

extern "C" float * get_frame(void){
	int retries = 6;
	int subpage;
	bool subpages[2] = {0,0};

	// Sync up with the cameras framerate by throwing data away
	while(retries--){
		auto start = std::chrono::system_clock::now();
		MLX90640_GetFrameData(MLX_I2C_ADDR, frame);
		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		std::this_thread::sleep_for(std::chrono::microseconds(frame_time - elapsed));
	}
	
	retries=10;

	while (retries-- && (!subpages[0] || !subpages[1])){
		printf("Retries: %d \n", retries);
		auto start = std::chrono::system_clock::now();
		
		MLX90640_GetFrameData(MLX_I2C_ADDR, frame);
		printf("Got data for page %d\n", MLX90640_GetSubPageNumber(frame));
		subpage = MLX90640_GetSubPageNumber(frame);

		subpages[subpage] = 1;

		printf("Converting data for page %d\n", subpage);

		eTa = MLX90640_GetTa(frame, &mlx90640);
		MLX90640_CalculateTo(frame, &mlx90640, emissivity, eTa, mlx90640To);
	

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		std::this_thread::sleep_for(std::chrono::microseconds(frame_time - elapsed));
	}
	printf("Finishing\n");

	return mlx90640To;
}

int main()
{
	setup(16);
	get_frame();
	return 0;
}
