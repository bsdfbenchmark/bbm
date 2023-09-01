Requirements
============

The minimum requirements of BBM that supports the built-in native backbone
(without packets or autodiff) and without python bindings are:

* cmake (version >= 3.21)
* C++20 compatible compiler (gcc >= 11 or clang >= 14)

Additional optional requirements are:

* pybind11: for including python bindings
* enoki: for CPU based packet and autodiff (experimental) support; 
* DrJIT: for CPU and LLVM based autodiff (experimental) support; 
* doxygen: for API document generation
* sphinx and sphinx_rtd_theme: for generating this documentation

The easiest way to include external libraries such as enoki or DrJIT is to
clone them from their respective repositories into the ``ext`` subdirectory in
the bbm main directory.
