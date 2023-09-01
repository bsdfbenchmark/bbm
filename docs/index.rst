.. BBM: A BRDF Benchmark documentation master file
    
BBM: A BRDF Benchmark
=====================

BBM is a ``C++20`` based BRDF Benchmark that aims to provide:

* Reference implementations of BSDF models
* Support and integrating reading measured BSDFs
* Support BSDF parameter fitting with different fitting methods and metrics
* Easy integration of published fitted BSDF parameters
* Integration in existing Rendering engines
* Bindings to Python
* Support different backbone engines (e.g., enoki, drjit, etc.)

BBM is a templated based library written in ``C++20`` relying on features such
as concepts. It has been tested on recent versions of ``GCC`` and
``Clang``. BBM uses ``cmake`` for makefile generation and ``sphinx+doxygen``
for documentation generation.
  
BBM is still under development. BBM currently (version 0.5.1, August 2023) supports:

* 34 BSDF and variants reference implementations (evaluation and importance sampling)
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
  - partial: DrJIT python bindings (currently requires manually importing the corresponding drjit.<scalar/llvm> python library)
  - not yet supported

    + DrJIT CUDA backbone
    + packet types

The development of BBM was in part supported by a National Science
Foundation grant: CNS-1823154.
      
.. toctree::
   :hidden:

   self

.. toctree::
   :maxdepth: 1
   :caption: Getting Started
   :hidden:

   source/requirements
   source/first_project
   source/config
   source/bbm_basics
   source/roadmap
   
.. toctree::
   :maxdepth: 1
   :caption: BSDF
   :hidden:

   source/use_bsdf
   source/serialization
   source/new_bsdfmodel
   source/new_microfacet
   source/precompute
   source/tools

.. toctree::
   :maxdepth: 1
   :caption: BSDF Fitting
   :hidden:

   source/fitting

.. toctree::
   :maxdepth: 1
   :caption: BBM Internals
   :hidden:

   source/internals
   source/stringconvert
   source/attribute
   source/args
   source/reflection
   source/bsdf_export

.. toctree::
   :maxdepth: 1
   :caption: API
   :hidden:
              
   source/concepts
   source/annotated
   source/files
   



      

   
