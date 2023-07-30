#if defined(BBM_PYTHON) && !defined(_BBM_NATIVE_PYTHON_H_)
#define _BBM_NATIVE_PYTHON_H_

#include <iostream>

#include <pybind11/numpy.h>
#include "backbone/array.h"

namespace pybind11 {
  namespace detail {

    template<typename ARR>
    struct type_caster<ARR, std::enable_if_t<backbone::is_array_v<ARR>>>
    {
      using array_type = std::decay_t<ARR>;
      using value_type = typename array_type::value_type;
    public:
      
      bool load(handle src, bool convert)
      {
        // check if py object == none
        if(src.is_none())
        {
          is_none = true;
          return true;
        }

        // if not convert, check type matches
        if(!isinstance<array_t<array_type>>(src) && !convert) return false;

        // setup interpretation of the array
        array arr = reinterpret_borrow<array>(src);
        arr = array_t<value_type, array::c_style>::ensure(arr);
        if(!arr) return false;

        // check shape of array
        if(arr.ndim() != 1 && convert) return false;
        if(arr.shape()[0] != value.size()) return false;

        // copy data
        const value_type* buf = static_cast<const value_type*>(arr.data());
        for(size_t i=0; i < value.size(); ++i)
          value[i] = buf[i];

        return true;
      }

      static handle cast(const array_type* src, return_value_policy policy, handle parent)
      {
        if(!src) return pybind11::none();
        return cast(*src, policy, parent);
      }
      
      static handle cast(const array_type& src, return_value_policy /*policy*/, handle /*parent*/)
      {
        std::array<size_t, 1> shape{src.size()};
        std::array<size_t, 1> stride{sizeof(value_type)};

        array arr(pybind11::dtype::of<value_type>(),
                  std::vector<ssize_t>(shape.begin(), shape.end()),
                  std::vector<ssize_t>(stride.begin(), stride.end()));

        value_type* buf = static_cast<value_type*>(arr.mutable_data());
        for(size_t i=0; i < src.size(); ++i)
          buf[i] = src[i];
        
        return arr.release();
      }

      template<typename T> using cast_op_type = pybind11::detail::cast_op_type<T>;
      
      static constexpr auto name = _("numpy.ndarray[dtype=") + npy_format_descriptor<value_type>::name + _("], shape=(") + _<backbone::array_size<array_type>>() + _(")");

      operator array_type*() { if (is_none) return nullptr; else return &value; }
      operator array_type&() { if (is_none) throw pybind11::cast_error("Cannot cast None or nullptr to an backbone::array."); return value; }
      
    private:
      array_type value;
      bool is_none = false;
    };

  } // end detail namespace
} // end pybind11 namespace


#endif /* _BBM_NATIVE_PYTHON_H_ */
