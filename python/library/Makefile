PYTHON := python3

python_version_str := $(shell ${PYTHON} --version 2>&1)
python_version_full := $(wordlist 2,4,$(subst ., ,${python_version_str}))
python_version_major := $(word 1, ${python_version_full})

ifeq (${python_version_major}, 3)
	SWIG_ARGS := -py3
endif

all:
	@echo ""
	@echo "Using: ${python_version_str} (${PYTHON})"
	@echo ""
	@echo "make build PYTHON=<python binary>   - Build using specified Python"
	@echo "make install PYTHON=<python binary> - Install using specified Python"
	@echo ""

build:
	swig3.0 ${SWIG_ARGS} -c++ -python MLX90640.i
	${PYTHON} setup.py build

install:
	${PYTHON} setup.py install

clean:
	rm -rf build/
	rm -rf dist/
	rm -rf MLX90640.egg-info/
	rm -f MLX90640.py*
	rm -f MLX90640_wrap.cxx
	rm -f MLX90640_wrap.c*
