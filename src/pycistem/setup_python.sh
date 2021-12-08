PYTHON_BUILD_DEFINITIONS="./pybuild/" bash ./pybuild/python-build 3.9.6  $(pwd)/python/
./python/bin/python -m venv pycistem-venv
pycistem-venv/bin/python -m pip install -r requirements.txt 