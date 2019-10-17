all:
	swig3.0 -c++ -python MLX90640.i
	python setup.py build

python3:
	swig3.0 -py3 -c++ -python MLX90640.i
	python3 setup.py build

install:
	python setup.py install

install3:
	python3 setup.py install

clean:
	rm -rf build/
	rm -rf dist/
	rm -rf MLX90640.egg-info/
	rm -f MLX90640.py*
	rm -f MLX90640_wrap.cxx
	rm -f MLX90640_wrap.c*