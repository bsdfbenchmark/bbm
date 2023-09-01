#ifndef _BBM_PY_CAST_H_
#define _BBM_PY_CAST_H_

#include "pybind11/numpy.h"

#include "util/typestring.h"
#include "util/attribute_value.h"

/************************************************************************/
/*! \file py_cast.h

  \brief Wrapper around py::cast for more robust casting to attributes and to
         aggregate types such as vec2d, vec3d, spectrum, etc...
*************************************************************************/

namespace bbm {
  namespace python {

    /********************************************************************/
    /*! \brief Default casting
     ********************************************************************/
    template<typename TARGET>
      struct py_cast
    {
      template<typename T>
        static inline TARGET cast(T&& t)
      {
        try
        {
          if constexpr (requires(T&& t) {{py::cast<TARGET>(std::forward<T>(t))};}) return py::cast<TARGET>(std::forward<T>(t));
          else throw std::runtime_error("BBM: casting error");
        }
        catch(...)
        {
          throw std::runtime_error(std::string("BBM: do not know how to cast to ") + std::string(bbm::typestring<TARGET>) + ".");
        }
      }
    };

    /********************************************************************/
    /*! \brief specialization for bbm::attribute
     ********************************************************************/
    template<typename TARGET> requires concepts::attribute<TARGET>
      struct py_cast<TARGET>
    {
      template<typename T>
        static inline TARGET cast(T&& t)
      {
        return py_cast<typename std::decay_t<TARGET>::type>::cast(std::forward<T>(t));
      }
    };

    /********************************************************************/
    /*! \brief specialization for aggregate types like vec2d, vec3d, spectrum, ...
     ********************************************************************/
    template<typename TARGET> requires (!std::same_as<value_t<TARGET>, std::decay_t<TARGET>> && !concepts::attribute<TARGET>) 
      struct py_cast<TARGET>
    {
    private:

      //! \brief copy constant values (val) in 'result'
      template<typename T>
        static void from_const(T& result, const scalar_t<TARGET>& val)
      {
        if constexpr (std::same_as<value_t<T>, std::decay_t<T>>) result = val;
        else for(auto& r : result) from_const(r, val);
      }

      //! \brief recusively copy values from the sub-array at partial indices [idx...] to result
      template<typename T, typename... Idx> 
        static void from_py_array(T& result, const py::array_t<scalar_t<TARGET>>& parr, Idx... idx)
      {
        // base case: T is not a aggregate type
        if constexpr (std::same_as<value_t<T>, std::decay_t<T>>)
        {
          // special case: end of array reached
          if((sizeof...(Idx) == parr.ndim())) return from_const(result, parr.at(idx...));
          
          // check size (must be last dimension with only 1 element)
          size_t size = parr.shape()[sizeof...(Idx)];
          if(sizeof...(Idx)+1 == parr.ndim() && size == 1) return from_const(result, parr.at(idx..., 0));

          // error
          throw bbm_size_error;
        }

        // recursion
        else
        {
          // check dimensions
          if(parr.ndim() <= ssize_t(sizeof...(Idx))) throw bbm_size_error;

          // check size
          size_t size = parr.shape()[sizeof...(Idx)];
          if(size != bbm::size(result)) throw bbm_size_error;

          // copy multiple data entries
          for(size_t current_idx = 0; current_idx != bbm::size(result); ++current_idx)
          {
            // special case: end of array reached
            if(parr.ndim() == sizeof...(Idx)+1) from_const( value(*std::next(bbm::begin(result), current_idx)), parr.at(idx..., current_idx));
            
            // recurse
            else from_py_array(value(*std::next(bbm::begin(result), current_idx)), parr, idx..., current_idx);
          }
        }

        // Done.
      }

    public:
      template<typename T>
        static inline TARGET cast(T&& t)
      {
        // try casting to TARGET first
        try
        {
          return py::cast<TARGET>(std::forward<T>(t));
        } catch(...) {}

        // try casting to scalar_t instead of TARGET
        try {
          if constexpr (std::constructible_from<TARGET, scalar_t<TARGET>>)
                         return TARGET( py_cast<scalar_t<TARGET>>::cast(std::forward<T>(t)) );
        } catch(...) {}

        // final attempt; try casting to py::array
        auto parr = py::cast<py::array_t<scalar_t<TARGET>>>(std::forward<T>(t));
        TARGET result;
        from_py_array(result, parr);
        return result;
      }
    };
    
  } // end python namespace
} // end bbm namespace

#endif /* _BBM_PY_CAST_H_ */
