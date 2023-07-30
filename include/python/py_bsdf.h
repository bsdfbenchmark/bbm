#ifndef _BBM_PY_BSDF_H_
#define _BBM_PY_BSDF_H_

#include "pybind11/pybind11.h"
namespace py = pybind11;

#include "util/typestring.h"
#include "util/toString.h"
#include "python/py_arg.h"
#include "bbm/bsdf_ptr.h"
#include "bbm/bsdf.h"

/***********************************************************************/
/*! \file py_bsdf.h
    \brief Export a BSDF model to python 
************************************************************************/

namespace bbm {
  namespace python {

    /*******************************************************************/
    /*! \brief Provide a human readable string of an bbm::args type.

      Prints the name and default value of the bbm::arg, and optinally the
      type if TYPE is set to true.
    ********************************************************************/
    template<typename ARGS, bool TYPE=false> requires is_args_v<ARGS>
    std::string args_to_string(void)
    {
      std::string result;
      
      CONSTFOR(idx, ARGS::size,
      {
        using arg = std::decay_t<decltype(std::declval<ARGS>().template get<idx>())>;

        if constexpr (idx != 0) result += ", ";

        // add type (if requested)
        if constexpr (TYPE) result += toString(typestring<typename arg::type>) + " ";
        
        // add name (use <unnamed> if empty)
        if constexpr (arg::name.empty) result += "<unnamed>";
        else result += toString(arg::name);

        // add default value (if exists)
        if constexpr (std::is_constructible_v<arg>)
          result += std::string(" = ") + toString(arg().value());
      });

      // Done.
      return result;
    }
    
    /*******************************************************************/
    /*! \brief export a BSDF model to Python 

        \tparam BSDFMODEL = BSDF model to export
        \param m = python module

        Creates a bsdf_ptr wrapper for a BSDF __model__, and exports it to the
        python module 'm' with 'name'. Using a trampoline function to set the
        attributes instead of direcly defining the attributes and default
        values, otherwise there might be issues with embedding the module more than
        once for certain types of default values (e.g., numpy).
     *******************************************************************/
    template<typename BSDFMODEL> requires concepts::bsdfmodel<BSDFMODEL> && concepts::constructor<BSDFMODEL>
      void def_bsdf(py::module& m)
    {
      std::string name = bbm::toString(BSDFMODEL::name);
      
      // message string
      std::string msg = "Constructs: " + name + "(" + args_to_string<typename BSDFMODEL::constructor_args_t>() + ")";

      // default trampoline function
      m.def(name.c_str(), [](py::args args, py::kwargs kwargs)
      {
        // create bsdf_ptr
        return make_bsdf_ptr( create_object<BSDFMODEL>(args, kwargs) );
      }, msg.c_str());
    }

  } // end python namespace
} // end bbm namespace

#endif /* _BBM_BBMBSDF_H_ */
