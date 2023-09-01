#ifndef _BBM_PYTHON_INTERPRETER_H_
#define _BBM_PYTHON_INTERPRETER_H_

#ifndef BBM_PYTHON
  #error Fatal Error: Python Support not enabled for BBM (-DBBM_PYTHON=ON)
#endif /* BBM_PYTHON */

#include <cassert>

#include <pybind11/embed.h> 
namespace py = pybind11;

#include "util/macro_util.h"
#include "io/dynamic_library.h"

/***********************************************************************/
/*! \file bbm_python_interpreter.h
    \brief Python interpreter for BBM modules
************************************************************************/

namespace bbm {
  namespace python {

    /*******************************************************************/
    /*! \brief Python interpreter with BBM module imported.
     
      \tparam CONF = configuration of the BBM environment to use
    ********************************************************************/
    template<typename CONF> requires concepts::config<CONF>
      struct interpreter
    {
      //! \brief Constructor: initializes BBM python interface.
      interpreter(void)
      {
        // ensure the python dynamic library is loaded 
        _python = loadDynamicLibrary( BBM_STRINGIFY(BBM_PYTHON_LIBRARY) );
        assert(_python);
        
        // setup python
        py::initialize_interpreter();
        _scope = py::module::import("__main__").attr("__dict__");

        // import bbm
        std::string import_string = std::string("from ") + std::string(BBM_STRINGIFY(BBM_NAME)) + std::string("_") + std::string(get_config<CONF>::name) + std::string(" import *");
        py::exec(import_string, _scope);
      }
      
      //! \brief Destructor releases scope and python interpreter appropriately 
      ~interpreter(void)
      {
        _scope.release();
        py::finalize_interpreter();
        closeDynamicLibrary(_python);
      }

      /******************************************************************/
      /*! \brief Execute Python code within the BBM environment.

        \tparam RET_TYPE = type of the return type (need explicit casting)
        \param prog = string of python code to execute.
        \returns object of type RET_TYPE

        Usage Example:
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
        interpreter ii
        auto ptr = i.capture<BsdfPtr>("Lambertian()");
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Returns a BsdfPtr objects to a Lambertian bsdf.
      *******************************************************************/
      template<typename RET_TYPE>
        RET_TYPE capture(const std::string& prog) const
      {
        try {
          return py::eval(prog, _scope).cast<RET_TYPE>();
        } catch(...) { throw std::runtime_error("BBM: python expression \"" + prog + "\" did not return a valid BBM Bsdf."); }
      }

      /******************************************************************/
      /*! \brief Execute Python code within the BBM environment.

        \param prog = string of python code to execute.
        
        Usage Example:
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
        interpreter i;
        i.execute<floatRGB>("print(str(Lambertian()))");
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Prints the default Lambertian bsdf.
        
      *******************************************************************/
      void execute(const std::string& prog) const
      {
        try {
          py::eval(prog, _scope);
        } catch(...) { throw std::runtime_error("BBM: failed to execute python expression \"" + prog + "\""); }
      }
      
    private:
      dl_handle_t _python;
      py::object _scope;
    };


    /*******************************************************************/
    /*! \brief Helper function to execute a python string and capture the output
      
      \tparam RET_TYPE = type of the return value (must be specified manually)
      \tparam CONF = config of the bbm python lib.
      \param prog = string of python program to run
    ********************************************************************/
    template<typename RET_TYPE, typename CONF=get_config<RET_TYPE>> requires concepts::config<CONF>
      RET_TYPE capture(const std::string& prog)
    {
      interpreter<CONF> intp;
      return intp.template capture<RET_TYPE>(prog);
    }      

    /*******************************************************************/
    /*! \brief Helper function to execute a python string and ignore the output

      \tparam CONF = config of the bbm python lib
      \param prog = string of python program to run
    ******************************************************************/
    template<typename CONF> requires concepts::config<CONF>
      void execute(const std::string& prog)
    {
      interpreter<CONF> intp;
      intp.execute(prog);
    }
    
  } // end python namespace
} // end bbm namespace

#endif /* _BBM_PYTHON_INTERPRETER_H_ */

