I2C_MODE = RPI
I2C_LIBS = -lbcm2835

ifeq ($(I2C_MODE), LINUX)
	I2C_LIBS =
endif

all: examples

examples: test step fbuf interp video

libMLX90640_API.so: functions/MLX90640_API.o functions/MLX90640_$(I2C_MODE)_I2C_Driver.o
	$(CXX) -fPIC -shared $^ -o $@ $(I2C_LIBS)

libMLX90640_API.a: functions/MLX90640_API.o functions/MLX90640_$(I2C_MODE)_I2C_Driver.o
	ar rcs $@ $^
	ranlib $@

functions/MLX90640_API.o functions/MLX90640_RPI_I2C_Driver.o functions/MLX90640_LINUX_I2C_Driver.o : CXXFLAGS+=-fPIC -I headers -shared $(I2C_LIBS)

examples/test.o examples/step.o examples/fbuf.o examples/interp.o examples/video.o : CXXFLAGS+=-std=c++11

test step fbuf interp video hotspot : CXXFLAGS+=-I. -std=c++11

examples/lib/interpolate.o : CC=$(CXX) -std=c++11

hotspot: examples/hotspot.o examples/lib/fb.o libMLX90640_API.a
	$(CXX) -L/home/pi/mlx90640-library $^ -o $@ $(I2C_LIBS)

test: examples/test.o libMLX90640_API.a
	$(CXX) -L/home/pi/mlx90640-library $^ -o $@ $(I2C_LIBS)

step: examples/step.o libMLX90640_API.a
	$(CXX) -L/home/pi/mlx90640-library $^ -o $@ $(I2C_LIBS)

fbuf: examples/fbuf.o examples/lib/fb.o libMLX90640_API.a
	$(CXX) -L/home/pi/mlx90640-library $^ -o $@ $(I2C_LIBS)

interp: examples/interp.o examples/lib/interpolate.o examples/lib/fb.o libMLX90640_API.a
	$(CXX) -L/home/pi/mlx90640-library $^ -o $@ $(I2C_LIBS)

video: examples/video.o examples/lib/fb.o libMLX90640_API.a
	$(CXX) -L/home/pi/mlx90640-library $^ -o $@ $(I2C_LIBS) -lavcodec -lavutil -lavformat -lbcm2835

bcm2835-1.55.tar.gz:	
	wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.55.tar.gz

bcm2835-1.55: bcm2835-1.55.tar.gz
	tar xzvf bcm2835-1.55.tar.gz

bcm2835: bcm2835-1.55
	cd bcm2835-1.55; ./configure; make; sudo make install

clean:
	rm -f test step fbuf interp video
	rm -f examples/*.o
	rm -f examples/lib/*.o
	rm -f functions/*.o
	rm -f *.o
	rm -f *.so
	rm -f test
	rm -f *.a
