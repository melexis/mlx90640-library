#!/usr/bin/env python

import os
import sys

from setuptools import setup, Extension
from setuptools.command.build_py import build_py
from setuptools.command.sdist import sdist
from distutils.spawn import find_executable
from glob import glob


# Fix so that build_ext runs before build_py
# Without this, wiringpi.py is generated too late and doesn't
# end up in the distribution when running setup.py bdist or bdist_wheel.
# Based on:
#  https://stackoverflow.com/a/29551581/7938656
#  and
#  https://blog.niteoweb.com/setuptools-run-custom-code-in-setup-py/
class build_py_ext_first(build_py):
    def run(self):
        self.run_command("build_ext")
        return build_py.run(self)


# Make sure wiringpi_wrap.c is available for the source dist, also.
class sdist_ext_first(sdist):
    def run(self):
        self.run_command("build_ext")
        return sdist.run(self)

classifiers = ['Development Status :: 4 - Beta',
               'Operating System :: POSIX :: Linux',
               'License :: OSI Approved :: MIT License',
               'Intended Audience :: Developers',
               'Programming Language :: Python :: 3',
               'Topic :: Software Development',
               'Topic :: System :: Hardware']

_MLX90640 = Extension(
    '_MLX90640',
    include_dirs=['../headers'],
    sources=['../functions/MLX90640_API.cpp', '../functions/MLX90640_LINUX_I2C_Driver.cpp', 'cffi_wrapper.cpp']
)

setup(
    name = 'MLX90640',
    version = '0.1.0',
    classifiers = classifiers,
    ext_modules = [ _MLX90640 ],
    py_modules = ["MLX90640"],
    install_requires=['cffi', 'numpy'],
    cmdclass = {'build_py' : build_py_ext_first, 'sdist' : sdist_ext_first},
)
