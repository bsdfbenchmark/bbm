Compile BBM
===========

BBM Library Compilation
-----------------------

The following assumes that BBM was successfully checked out from github.  As a
first step, compile the tools provided with BBM to validate that all
requirements are present.  After entering the BBM directory:

.. code-block:: bash
                
               mkdir build
               cd build
               cmake .. --preset=gcc -DBBM_BINARIES=ON
               make

If all requirements are met, this should by default build only the binary
tools provided with BBM.  If compilation was successful then the following
command should provide basic information about BBM:

.. code-block:: bash
                
               ./bbm_info

Version 0.5.0 of BBM outputs:

.. code-block:: none

  BBM_NAME = 'bbm' using 'native' backbone and compiled with NO python support.
  31 BSDF models supported:
  + AshikhminShirley
  + AshikhminShirleyFull
  + Bagher
  + CookTorrance
  + CookTorranceHeitz
  + CookTorranceWalter
  + GGX
  + GGXHeitz
  + He
  + HeWestin
  + HeHolzschuch
  + Lafortune
  + Lambertian
  + LowMicrofacet
  + LowMicrofacetSup
  + LowSmooth
  + NganWard
  + NganWardDuer
  + NganBlinnPhong
  + NganLafortune
  + NganCookTorrance
  + NganAshikhminShirley
  + NganHe
  + OrenNayar
  + Phong
  + PhongWalter
  + Ribardiere
  + RibardiereAnisotropic
  + Ward
  + WardDuer
  + WardDuerGeislerMoroder
  1 static (measured) BSDFs supported:
  + Merl


Stand-alone BBM Project
-----------------------

In most cases you want to use BBM in a separate stand-alone project.  There
are two ways of including BBM: as a subproject in a subdirectory, or installed
externally.  The only difference between both cases is a few lines in
``CMakeLists.txt``.  We will demonstrate both cases using the same basic
``bbmtest.cpp`` program:

.. code-block:: c

   #include "bbm.h"

   int main(int argc, char** argv)
   {
      BBM_IMPORT_CONFIG( floatRGB );

      return 0;
   }
   
If BBM is installed in an external directory then the following ``CMakeLists.txt``
example will include BBM in your project:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.21)
   project(MY_PROJECT)
  
   find_package(BBM REQUIRED)
  
   add_executable(bbm_test bbm_test.cpp)
   target_compile_definitions(bbm_test INTERFACE bbm::core)
   target_include_directories(bbm_test PRIVATE bbm::core)
   target_link_libraries(bbm_test PRIVATE bbm::core)

You will need to inform cmake where to find the library by providing setting ``CMAKE_PREFIX_PATH``:

.. code-block:: bash

   mkdir build; cd build
   cmake .. --preset=gcc -DCMAKE_PREFIX_PATH=<BBM_DIR>/cmake
   make bbm_test


Note: you will need to copy CMakePresets.json from the BBM directory to your
project directory (or adjust your own presets accordingly).

If BBM is included as a subproject in a subdirectory ``bbm``, then we replace
``Find_Package(BBM REQUIRED)`` in the above ``CMakeLists.txt``:

.. code-block:: cmake

   set(CMAKE_CXX_STANDARD 20)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)

   add_subdirectory(bbm)
   add_library(bbm::core ALIAS ${BBM_NAME})

Note in both cases, BBM is compiled locally (if needed), so you do not need to
precompile the BBM library.
