
rm -r ./build ./mlx90640test _MLX90640.cpython-37m-arm-linux-gnueabihf.so 
mkdir ./build

gcc -pthread -Wno-unused-result -Wsign-compare -DNDEBUG -g -fwrapv -O3 -Wall -fPIC -I../headers -c ../functions/MLX90640_LINUX_I2C_Driver.cpp -o build/MLX90640_LINUX_I2C_Driver.o


gcc -pthread -Wno-unused-result -Wsign-compare -DNDEBUG -g -fwrapv -O3 -Wall -fPIC -I../headers -c ../functions/MLX90640_API.cpp -o build/MLX90640_API.o

gcc -pthread -Wno-unused-result -Wsign-compare -DNDEBUG -g -fwrapv -O3 -Wall -fPIC -I../headers -c mlx90640-python.cpp -o build/mlx90640-python.o

g++ -pthread build/MLX90640_LINUX_I2C_Driver.o build/MLX90640_API.o build/mlx90640-python.o -o ./mlx90640test
g++ -pthread -shared build/MLX90640_LINUX_I2C_Driver.o build/MLX90640_API.o build/mlx90640-python.o -o _MLX90640.cpython-37m-arm-linux-gnueabihf.so -fPIC 
