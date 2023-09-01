#include "pybind11/pybind11.h"

namespace py = pybind11;

#include "bbm.h"
#include "python/python_macro.h"

/***********************************************************************/
/*! \file bbm_python.cpp
    \brief Generate Python bindings

    This C++ source file expects two macros to be set in order to 
    function correctly:

    \param BBM_NAME = (macro) base name of the BBM python library
    \param BBM_CONFIG = (macro) config name to use as defined in bbm_config.h 

    The resulting python library will be named <BBM_NAME>_<BBM_CONFIG>
*************************************************************************/    

BBM_PYTHON_MODULE
{
  BBM_PYTHON_CORE
  #include "export/bbm_python.h"
  #include "bbm_bsdfmodels.h"
  #include "export/clear_export.h"
}

