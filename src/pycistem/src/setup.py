from distutils.core import setup
setup (name = 'pycistem',
       version = '0.1',
       author = "Johannes Elferich",
       description = """Install precompiled extension""",
       py_modules = ["pycistem"],
       packages=[''],
       package_data={'': ['../../../build/pybind11_Intel-gpu-debug/src/pycistem.cpython-39-x86_64-linux-gnu.so']},
       )