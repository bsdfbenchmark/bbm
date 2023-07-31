# BBM: A BSDF Benchmark

## About this project

BBM is a C++20 based BRDF Benchmark that aims to provide:

* Reference implementations of BSDF models
* Support and integrating reading measured BSDFs
* Support BSDF parameter fitting with different fitting methods and metrics
* Easy integration of published fitted BSDF parameters
* Integration in existing Rendering engines
* Bindings to Python
* Support different backbone engines (e.g., enoki, drjit, etc.)

BBM is a templated based library written in C++20 relying on features such
as concepts. It has been tested on recent versions of GCC and
Clang. BBM uses cmake for makefile generation and sphinx+breathe+doxygen
for documentation generation.
  
BBM is still under development. BBM currently (version 0.5.0, July 2023) supports:

* 31 BSDF and variants reference implementations (evaluation and importance sampling)
* support for reading the MIT-MERL measured BSDFs
* python bindings for BSDF models
* a plugin for Mitsuba 1 (RGB and homogeneous BSDFs only)
* 6 different fitting metrics and BSDF fitting support (beta)
* a native backbone implementation that does not rely on external libraries
  (no autodiff or packet support)
* support for the Enoki backbone

  - fully supported: scalar and packet types
  - experimental: CPU-based autodiff
  - not supported: GPU types
* support for the DrJIT backbone

  - fully supported: scalar and LLVM DrJit backbone for non-differentiable types
  - experimental: CPU and LLVM based autodiff
  - not supported

    + DrJIT CUDA backbone
    + packet types

The development of BBM was in part supported by a National Science
Foundation grant: CNS-1823154.

## Getting Started

The basic prerequisites are: cmake (>= 3.21) and a C++20 compatible compiler.

```
mkdir build
cd build
cmake .. -DBBM_BINARIES=ON
make
```

## Documentation

BBM relies on sphinx (>= 6.2) + breathe + doxygen (>= 1.9.4) to generate documentation.  If all three are installed, and the above cmake makefile generation was succesful, then running the following in the "build" directory:

```
make docs
```

will produce "docs/html/index.html" in the "build" directory.

You can also find the latest version of the documents on [ReadTheDocs](https://bbm.readthedocs.io/en/latest/index.html).

## Support

This project was in part supported by the National Science Foundation (CNS-1823154).
