#ifndef _BBM_EMBED_MODULE_H_
#define _BBM_EMBED_MODULE_H_

#include "bbm.h"
#include "python/python_macro.h"
#include "python/bbm_python_interpreter.h"


/***********************************************************************/
/*! \file bbm_embed_module.h
    \brief Embed a static python module in the executable.

    Requires BBM_NAME and BBM_CONFIG to be set.  The module is automatically
    embedded when included, and the bbm::embed namespace contains shorthand
    for accessing the embedded bbm python module.  

************************************************************************/

namespace detail {
  BBM_PYTHON_EMBED_MODULE
  {
    BBM_PYTHON_CORE
    #include "export/bbm_python.h"
    #include "bbm_bsdfmodels.h"
    #include "export/clear_export.h"
  }
} // end detail namespace


namespace bbm {
  namespace embed {

    //! \brief Shorthand for the embedded config
    using config = bbm::BBM_CONFIG;

    //! \brief Shorthand for the interpreter operating on the embedded config
    using interpreter = bbm::python::interpreter<config>;

    //! \brief Shorthand for the capture method on the embedded config
    template<typename RET_TYPE>
      inline RET_TYPE capture(const std::string& prog) { return bbm::python::capture<RET_TYPE, config>(prog); }

    //! \brief Shorthand for the execute method on the embedded config
    inline void execute(const std::string& prog) {  bbm::python::execute<config>(prog); }

  } // end embed namespace
} // end bbm namespace

#endif /* _BBM_EMBED_MODULE_H_ */
