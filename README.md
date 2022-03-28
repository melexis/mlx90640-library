# mlx90640-library
### Arthur: [Xiangmin Xu][Haiyang You](https://github.com/Maripoforest)
MLX90640 library, modified for raspberry pi model 4B.

## Prerequisite
Make sure the LINUX I2C devlib is installed.
```
sudo apt-get install libi2c-dev
```

## How do use:
```
cd mymlx90640
mkdir build && cd build
cmake ..
make
../sensor
```
This method is based on console output, if you want to use the Qt output, please go to rep [Smart-Light-Bulb-Control-with-Gesture](https://github.com/Maripoforest/Smart-Light-Bulb-Control-with-Gesture.git) and follow the instructions there.

## Update
2022.3.14 Update, compatible for RPI Model 4B.

2022.3.28 Update, make all function in class, now can be started with wrapped functions. The whole driver is now event-driven.

2022@[Xiangmin Xu](https://github.com/Maripoforest).

## Contributors
This library is owned by: [Melexis Open Source Team](https://github.com/melexis)

And is forked from: [Pimoroni Ltd](https://github.com/pimoroni)
 
