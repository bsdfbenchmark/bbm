BBM Configuration
-----------------

Previously, when compiling the binaries in the BBM library,
``-DBBM_BINARIES=ON`` was added when invoking cmake. This option enables the
compilation of the BBM binary tools.  Other options are available too; for
convenience, BBM's ``CMakeLists.txt`` imports a ``config.cmake`` file from the
project root directory (if available).  The location of ``config.cmake`` can
also be specified by passing ``-DBBM_CONFIG=<file>`` when invoking cmake.

A default config file template ``config.cmake.template`` is provided in the
BBM source directory with comments explaining each available option.  The most
common ones are:


``BBM_NAME``
~~~~~~~~~~~~

This is the base name of the library and it is used for setting up the default
compile target and to determine the name of python modules (if requested).  By
default this option takes the name of the BBM source directory name.  The
template sets this to ``bbm``.

``BBM_BACKBONE``
~~~~~~~~~~~~~~~~

BBM can be compiled with different backbones that take care of the low-level
operations.  Currently there are three backbones supported: ``native`` (scalar
operations only; no autodiff or packets), ``enoki`` (scalar and packet
operations; experimental support for CPU autodiff), and ``drjit`` (scalar
operations and experimental CPU or LLVM autodiff support).  Note, this assumes
that the supporting backbone libraries are correctly configured (see options
below).  The default is ``native`` which is a stand-alone backbone that does
not require external libraries.

``BBM_BINARIES``
~~~~~~~~~~~~~~~~

As noted above, this boolean option enables/disables the compilation of tools included
with BBM.

``BBM_PYTHON``
~~~~~~~~~~~~~~

This boolean option will enable embedded python support.  Note this will not produce
the python libraries (see ``BBM_PYTHON_LIBRARIES``). This requires ``pybind11`` to be installed.

``BBM_PYTHON_LIBRARIES``
~~~~~~~~~~~~~~~~~~~~~~~~

This boolean option will compile python libraries of BBM.  Multiple version will be
compiled (see ``BBM_PYTHON_CONFIG``).  This requires ``pybind11`` to be installed.

``BBM_PYTHON_CONFIG``
~~~~~~~~~~~~~~~~~~~~~

A list of string naming BBM configurations (see :ref:`bbm\:\:config`) to compile into a
separate python library.  Each library will be named
``${BBM_NAME}_${BBM_CONFIG}``.

``PYBIND11_PATH``
~~~~~~~~~~~~~~~~~

By default BBM will search for ``pybind11`` in the ``ext`` subdirectory in the BBM
source directory.  Set this variable if ``pybind11`` is not found.

``ENOKI_PATH`` and ``DRJIT_PATH``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Location of the ``enoki`` and ``drjit`` library.  By default the ``ext`` subdirectory is
searched.

``ENOKI_AUTODIFF`` and ``DRJIT_AUTODIFF``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Boolean options to enable/disable the inclusion of Enoki's or DrJIT's (CPU) autodiff functionality.

``DRJIT_JIT``
~~~~~~~~~~~~~

Boolean option to enable/disable DrJIT's just-in-time functionality.

``DRJIT_BACKBONE``
~~~~~~~~~~~~~~~~~~

DrJIT supports multiple backbones itself. Currently, ``scalar`` is supported,
and ``LLVM`` support is experimental.  The ``CUDA`` backbone is currently not
supported.

``MITUSBA_EXPORTER``
~~~~~~~~~~~~~~~~~~~~

Boolean option to enable the creation of an Mitsuba module for including
(homogeneous) BBM BSDFs in Mistuba (0.6).  Additional options related to the
Mitsuba exporter are:

* ``MITSUBA_PATH``: by default the same level as the BBM library is searched.
* ``MITSUBA_BINARY_PATH``: path to where to copy the module (default all
  directories in ``MITSUBA_PATH`` are searched.
* ``MITSUBA_COMPILER_OPTIONS``: BBM tries to guess the compiler options from
  the files in ``MITSUBA_PATH``.  Use this option to override.

