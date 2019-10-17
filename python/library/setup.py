#!/usr/bin/env python

import os
import sys

from setuptools import setup, Extension
from setuptools.command.build_py import build_py
from setuptools.command.sdist import sdist
from distutils.spawn import find_executable
from glob import glob

sources = ['mlx90640-python.cpp']
# If we have swig, use it.  Otherwise, use the pre-generated
# wrapper from the source distribution.
if find_executable('swig'):
    sources += ['MLX90640.i']
elif os.path.exists('MLX90640_wrap.cxx'):
    sources += ['MLX90640_wrap.cxx']
elif os.path.exists('MLX90640_wrap.c'):
    sources += ['MLX90640_wrap.c']
else:
    print("Error:  Building this module requires either that swig is installed\n"
          "        (e.g., 'sudo apt install swig') or that MLX90640_wrap.c is available.\n")
    sys.exit(1)

# Fix so that build_ext runs before build_py
# Based on:
#  https://stackoverflow.com/a/29551581/7938656
#  and
#  https://blog.niteoweb.com/setuptools-run-custom-code-in-setup-py/
class build_py_ext_first(build_py):
    def run(self):
        self.run_command("build_ext")
        return build_py.run(self)


# Make sure MLX90640_wrap.c is available for the source dist, also.
class sdist_ext_first(sdist):
    def run(self):
        self.run_command("build_ext")
        return sdist.run(self)

classifiers = ['Development Status :: 4 - Beta',
               'Operating System :: POSIX :: Linux',
               'License :: OSI Approved :: MIT License',
               'Intended Audience :: Developers',
               'Programming Language :: Python :: 2.6',
               'Programming Language :: Python :: 2.7',
               'Programming Language :: Python :: 3',
               'Topic :: Software Development',
               'Topic :: System :: Hardware']

_MLX90640 = Extension(
    '_MLX90640',
    #We now require the base library to be installed, hence: no special header dirs needed!
    #include_dirs=['../../headers'],
    sources=sources,
    swig_opts=['-threads'],
    #extra_link_args=['-lbcm2835']
    #instead of using bcm2835 I2C driver all the time, use the one linked into the library
    extra_link_args=['-lMLX90640_API']
)

setup(
    name = 'MLX90640',
    version = '0.0.2',
    classifiers = classifiers,
    ext_modules = [ _MLX90640 ],
    py_modules = ["MLX90640"],
    install_requires=[],
    cmdclass = {'build_py' : build_py_ext_first, 'sdist' : sdist_ext_first},
)
