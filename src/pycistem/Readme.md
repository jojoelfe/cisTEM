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


# How to compile

- Run configure like this:

CC=icc CXX=icpc ../../configure --enable-experimental --enable-python --with-cuda=${cuda_dir} --disable-staticmode --enable-openmp  --with-wx-config=/groups/elferich/wx/wxWidgets-3.0.5/wx-config

- After compile of cistem is finished, the python module can be built like this (the stuff in autoconf doesn't work yet)

icpc `~/wx/wxWidgets-3.0.5/wx-config --cxxflags` -shared -DEXPERIMENTAL `../../../src/pycistem/pycistem-venv/bin/python -m pybind11 --includes` -O2 -o pycistem.cpython-39-x86_64-linux-gnu.so ../../../src/pycistem/src/main.cpp `../../../src/pycistem/python/bin/python-config --ldflags` -fPIC -L/groups/elferich/wx/wxWidgets-3.0.5/lib -pthread   -Wl,-rpath,/groups/elferich/wx/wxWidgets-3.0.5/lib -lwx_gtk2u_xrc-3.0 -lwx_gtk2u_html-3.0 -lwx_gtk2u_qa-3.0 -lwx_gtk2u_adv-3.0 -lwx_gtk2u_core-3.0 -lwx_baseu_xml-3.0 -lwx_baseu_net-3.0 -lwx_baseu-3.0 core/libcore*.o core/sqlite/libcore_a-sqlite3.o core/json/libcore_a-json*.o -ltiff  -fopenmp -lrt -ltiff -lz -pthread -lfftw3 -mkl=rt -lmkl_rt

- It seems to be fine to compiel python with gcc and cistem and the module with icpc

- Compile wx with shared
- configure with -enable-shared
- Add -fPIC
- Add -lz
- Don't use Debug mode