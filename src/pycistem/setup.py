import sys

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension
from setuptools import setup
from setuptools.command.build_ext import build_ext

__version__ = "0.0.1"
__compiler__ = "g++"
__WX_FLAGS__ = ""
__CPP_FLAGS__ = ""


with open("config.log","r") as config_log:
    configs = config_log.readlines()

for line in configs:
    if line.startswith("#define CISTEM_VERSION_TEXT"):
        __version__ = line.split(" ")[-1][1:-2]
    if line.startswith("CXX="):
        __compiler__ = line[5:-2]
    if line.startswith("WX_CPPFLAGS_BASE="):
        __WX_FLAGS__ = line[18:-2]
    if line.startswith("CPPFLAGS="):
        __CPP_FLAGS__ = line[10:-2]
        


class custom_build_ext(build_ext):
    def build_extensions(self):
        # Override the compiler executables. Importantly, this
        # removes the "default" compiler flags that would
        # otherwise get passed on to to the compiler, i.e.,
        # distutils.sysconfig.get_var("CFLAGS").
        self.compiler.set_executable("compiler_so", __compiler__ + " " + __WX_FLAGS__ + " " + __CPP_FLAGS__)
        self.compiler.set_executable("compiler_cxx", __compiler__ + " " + __WX_FLAGS__ + " " + __CPP_FLAGS__)
        self.compiler.set_executable("linker_so", __compiler__ + " " + __WX_FLAGS__ + " " + __CPP_FLAGS__)
        build_ext.build_extensions(self)

ext_modules = [
    Pybind11Extension("python_example",
        ["../../src/pycistem/src/main.cpp"],
        # Example: passing in the version to the compiled code
        define_macros = [('VERSION_INFO', __version__)],
        extra_compile_args = ["-fPIC"],
        ),
]

setup(
    name="pycistem",
    version=__version__,
    author="Johannes Elferich",
    author_email="jojotux123@hotmail.com",
    url="https://github.com/jojoelfe/cisTEM",
    description="Python binding for the cisTEM core library",
    long_description="",
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": custom_build_ext},
    zip_safe=False,
    python_requires=">=3.8",
)

