#ifndef _BBM_PYTHON_MACRO_H_
#define _BBM_PYTHON_MACRO_H_

#include "util/macro_util.h"
#include "python/py_core.h"
#include "python/py_bsdf.h"

/***********************************************************************/
/*! \file python_macro.h
    \brief Shorthand definitions for a python modules

    Expects BBM_NAME and BBM_CONFIG are set.  Usage:

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
    #include "bbm.h"
    BBM_PYTHON_<EMBED_>MODULE
    {
        BBM_PYTHON_CORE
        #include "export/bbm_python.h"
        #include "bbm.h"
        #include "export/clear_export.h"
    }
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The generated module is named <BBM_NAME>_<BBM_CONFIG>. The difference
    between BBM_PYTHON_MODULE and BBM_PYTHON_EMBED_MODULE is that the former
    generates a dynamic library (that can be imported in python) while the
    latter one embeds the module in the resulting executable.
************************************************************************/

/************* Check Existence of BBM_CONFIG and BBM_NAME **************/ 
#ifndef BBM_CONFIG
  #error Fatal Error: missing configuration name (-DBBM_CONFIG=<name>)
#endif /* BBM_CONFIG */

#ifndef BBM_NAME
  #error Fatal Error: missing library name (-DBBM_NAME=<name>)
#endif /* BBM_NAME */
/***********************************************************************/

//! \brief concat the BBM_NAME and BBM_CONFIG
#define BBM_PYTHON_MODULE_NAME BBM_CALL(_BBM_UCAT, BBM_NAME, BBM_CONFIG)
#define _BBM_UCAT(a, b) a ## _ ## b

//! \brief Create a python module (dynamic library) named BBM_PYTHON_MODULE_NAME
#define BBM_PYTHON_MODULE        BBM_CALL(PYBIND11_MODULE, BBM_PYTHON_MODULE_NAME, m)

//! \brief Create a python module (embedded in the object file) named BBN_PYTHON_MODULE_NAME
#define BBM_PYTHON_EMBED_MODULE  BBM_CALL(PYBIND11_EMBEDDED_MODULE, BBM_PYTHON_MODULE_NAME, m)

//! \brief Import the core bbm python functionality 
#define BBM_PYTHON_CORE          bbm::python::core<bbm::BBM_CONFIG>(m);

#endif /* _BBM_PYTHON_MACRO_H_ */
