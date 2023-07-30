#ifndef _BBM_PY_ATTRIBUTE_H_
#define _BBM_PY_ATTRIBUTE_H_

#include <utility>
#include <stdexcept>

#include "concepts/constructor.h"

/************************************************************************/
/*! \file py_arg.h
  \brief Create an object with a bbm constructor (with bbm::args) from
  py::args and py::kwargs arguments.
*************************************************************************/

namespace bbm {
  namespace python {

    /** Implementation detail for py_args_to_bbm_args ***/
    namespace detail {

      /******************************************************************/
      /*! \brief Extract the value from py::args and py::kwargs matching a bbm::arg
       ******************************************************************/
      template<typename ARG, size_t IDX> requires is_arg_v<ARG>
      inline decltype(auto) py_args_to_bbm_arg(py::args& args, py::kwargs& kwargs)
      {
        using arg_t = std::decay_t<ARG>;
        using type = std::decay_t<typename arg_t::type>;

        // check if name exists
        if(kwargs.size() > 0 && kwargs.contains( arg_t::name.value ))
        {
          return arg_t( py::cast<type>( kwargs[ arg_t::name.value ] ) );
        }

        // otherwise copy from arg if available
        else if(args.size() > IDX)
        {
          return arg_t( py::cast<type>(args[IDX]) );
        }

        // otherwise return default value?
        else if constexpr (std::is_constructible_v<arg_t>)
        {
          return arg_t();
        }

        // else fail
        else throw std::runtime_error(std::string("BBM: value required for argument '") + toString(arg_t::name) + "'.'");

        // unreachable...
        return arg_t(type());
      }

      /******************************************************************/
      /*! \brief Create a bbm::args from py_args and py::kwargs
       ******************************************************************/
      template<typename ARGS, size_t... IDX> requires is_args_v<ARGS>
        inline auto py_args_to_bbm_args(py::args& args, py::kwargs& kwargs, std::index_sequence<IDX...>)
      {
        return ARGS( py_args_to_bbm_arg< decltype(std::declval<ARGS>().template get<IDX>()), IDX >(args, kwargs)... );
      }


      /******************************************************************/
      /*! \brief Check if a name is in a bbm args at runtime
       ******************************************************************/
      template<typename ARGS, size_t... IDX> requires is_args_v<ARGS>
        inline bool is_valid_bbm_arg_name(const std::string& name, std::index_sequence<IDX...>)
      {
        return ((std::decay_t<typename ARGS::template type<IDX>>::name.value == name) || ...);
      }
      
    } // end detail namespace

      
    /********************************************************************/
    /*! \brief Create a bbm::arg from py:args and py::kwargs

      \tparam ARGS = bbm::args type to create and fill
      \param args = list of python arguments
      \param kwargs = list of keyword arguments
     ********************************************************************/
    template<typename ARGS> requires is_args_v<ARGS>
      inline auto py_args_to_bbm_args(py::args& args, py::kwargs& kwargs)
    {
      // check validity: number of py::args
      if(args.size() > ARGS::size) throw std::runtime_error(std::string("BBM: too many arguments (") + toString(args.size()) + "given, expected maximum: " + toString(ARGS::size) + ").");

      // check validity: names of py:kwargs
      for(auto& kwarg : kwargs)
      {
        std::string name = py::cast<std::string>(kwarg.first);
        if(!detail::is_valid_bbm_arg_name<ARGS>(name, std::make_index_sequence<ARGS::size>{})) throw std::runtime_error(std::string("BBM: invalid argument name '") + name + "'.");
      }
        
      // Extract args
      return detail::py_args_to_bbm_args<ARGS>(args, kwargs, std::make_index_sequence<ARGS::size>{});
    }

    
    /********************************************************************/
    /*! \brief Construct an object with a valid 'constructor_args_t' based
        on python args and kwargs

      \tparam OBJ_TYPE = object type to create
      \param args = list of python arguments (copied to the first N attributes)
      \param kwargs = list of keyword arguments

      Attempt to create an object with with 'OBJ_TYPE' from py::args and
      py::kwargs when 'constructor_ars_t' exists (i.e., bbm/constructor.h
      was used to declare a constructor).
    *********************************************************************/
    template<typename OBJ_TYPE> requires concepts::constructor<OBJ_TYPE>
      inline OBJ_TYPE create_object(py::args& args, py::kwargs& kwargs)
    {
      return OBJ_TYPE( py_args_to_bbm_args<typename OBJ_TYPE::constructor_args_t>(args, kwargs) );
    }

    
  } // end python namepace
} // end bbm namespace

#endif /* _BBM_PY_ATTRIBUTE_H_ */
