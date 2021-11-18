# Python integration of cisTEM

It would be great to integrate cisTEM with python so that:

- We could use python scripts instead of C++ binaries for some processing tasks
- We could use the core library from cisTEM, particularly the Image class, from python scripts

The plan to do this is to use pybind11 to compile a python module that exposes the Image class. For this we 
need python headers to compiel against and it woudl be great that these could be different from system python.
That way we could compile for several different cisTEM version and distribute binary wheels to PiPy. Also 
we could use the same mechanism to ship a python binary together with cisTEM in a way that does not interfere
with any system-wide python install. I also want to avoid conda, since it does so much magic I don't fully understand.

My current plan is to use the python-build script from pyenv (MIT license) to build python during the cisTEM compile process
and use this to provide cistem-python and cistem-napari binaries in teh cistem package and to use these python headers to 
compile a cistem python module.

So the process is:

- Build custom python with python-build (lives in src/pycistem/python)
- Create a venv where we install python packages we want to have (numpy,napari,pybind11 etc..) ( lives in src/pycistem/pycistem-venv)
- Use this venv during the normal build process to build pycistem module
