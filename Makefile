all: test

libMLX90640_API.so: functions/MLX90640_API.o functions/MLX90640_RPI_I2C_Driver.o
	$(CXX) -fPIC -shared $^ -o $@ -lbcm2835

libMLX90640_API.a: functions/MLX90640_API.o functions/MLX90640_RPI_I2C_Driver.o
	ar rcs $@ $^
	ranlib $@

functions/MLX90640_API.o functions/MLX90640_RPI_I2C_Driver.o : CXXFLAGS+=-fPIC -I headers -shared -lbcm2835

test: test.o libMLX90640_API.a
	$(CXX) -L/home/pi/mlx90640-library $^ -o $@ -lbcm2835

clean:
	rm -f *.o
	rm -f *.so
	rm -f test
	rm -f *.a
