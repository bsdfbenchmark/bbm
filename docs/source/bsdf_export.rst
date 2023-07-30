BBM_BSDF_EXPORT
===============


BBM requires a way to enumerate all BSDF models.  While it possible to have
programmers add the BSDF model to a static list, this would not be ideal.
First, it would require an instance of a BSDF model to be created.  This is
not ideal for measured BSDF models as this would require a default (i.e.,
empty) implementation.  Second, such a list would be created at run-time.

Instead BBM requires that a ``BBM_BSDF_EXPORT(...)`` macro call to be added
outside the include guards.  Initially the ``BBM_BSDF_EXPORT`` macro empty.
This empty macro is defined in `include/export/clear_export.h
<../doxygen/html/clear__export_8h_source.html>`_.  This include file is
included as part of ``include/bbm/bbm_core.h`` file that also
imports the backbone and the configurations and which is called by the
top-level ``include/bbm.h``.

.. note::

   BBM's cmake script auto-generates three header files. Two of these files are
   ''include/bbm_bsdfmodel.h`` and ``include/bbm_staticmodel.h`` that include
   all header files in the ``include/bsdfmodel`` and ``include/staticmodel``
   directories.

   The final file is ``include/bbm.h`` that import the previous two generated
   files and ``include/bbm/bbm_core.h``, and which is the file a programmer
   should include to setup bbm.


If, after the initial inclusion of the BSDF models, we include the BSDF models
again, then anything *inside* the guards will not be parsed.  However, the
``BBM_BSDF_EXPORT`` macro is called *outside*, and thus processed again.  This
allows to change this macro to perform an action on all BSDF models.

For example, in ``bbm/bbm_info.cpp`` we leverage this mechanism to collect the
names of all the available BSDF models:

.. code-block:: c++

  std::vector<std::string> bsdflist;
  #undef BBM_EXPORT_BSDFMODEL
  #define BBM_EXPORT_BSDFMODEL(bsdfmodel) bsdflist.push_back( bsdfmodel<bbm::floatRGB>::name.value );
  #include "bbm_bsdfmodel.h"
  #include "bbm_staticmodel.h"
  #include "export/clear_export.h"

This example, first creates a vector ``bsdflist`` to store all the names. Next
we undefine BBM_EXPORT_BSDFMODEL to avoid any clashes, followed by a
definition of a new exporter that pushes back the name of each model on
``bsdflist``.  Next we iterate through all models by including
``bbm_bsdfmodel.h`` and ``bbm_staticmodel.h``.  These two lines will do the
actual population of the vector.  Finally, we clear ``BBM_BSDF_EXPORT`` using
``export/clear_export.h``.  This last step is needed in case any of the models
is explicitely included again in another part of the code.

This same mechanism is also used to create the python library. A custom
``BBM_BSDF_EXPORT`` macro is defined in ``include/exporter/bbm_python.h``:

.. code-block:: c++

  #undef BBM_EXPORT_BSDFMODEL
  #define BBM_EXPORT_BSDFMODEL(bsdfmodel) bbm::python::def_bsdf<bsdfmodel<bbm::BBM_CONFIG>>(m);

Note, this assume that a ``pybind11`` module ``m`` has been setup (using the
macro ``BBM_PYTHON_MODULE``.

Please refer to `include/python/py_bsdf.h
<../doxygen/html/py__bsdf_8h_source.html>`_ and `include/python/py_arg.h
<../doxygen/html/py__arg_8h_source.html>`_ for the implementation details
of how BBM creates a python binding for each ``bsdfmodel`` given the
``constructor_arg_t`` typedef created by ``BBM_DEFAULT_CONSTRUCTOR``.

