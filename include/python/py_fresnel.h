#ifndef _BBM_PYTHON_FRESNEL_H_
#define _BBM_PYTHON_FRESNEL_H_

#include "core/ior.h"

/************************************************************************/
/*! \file py_fresnel.h
  \brief Export ior::ior and ior::reflectance to pyton
*************************************************************************/

namespace bbm {
  namespace python {

    /********************************************************************/
    /*! \brief Define a python interface for ior::ior and ior::reflectance
      given a value type T.

      \tparam T = value type of ior or reflectance

    ********************************************************************/
    template<typename T>
      void fresnel(py::module& m, const std::string& prefix = "")
    {
      py::class_<ior::ior<T>>(m, (prefix + "Ior").c_str())
        .def(py::init<const remove_diff_t<T>&>())
        .def(py::init<const ior::reflectance<T>&>())
        .def("__str__", [](const ior::ior<T>& ior) { return bbm::toString(ior); });

      py::class_<ior::reflectance<T>>(m, (prefix + "Reflectance").c_str())
        .def(py::init<const remove_diff_t<T>&>())
        .def(py::init<const ior::ior<T>&>())
        .def("__str__", [](const ior::reflectance<T>& refl) { return bbm::toString(refl); });

      py::implicitly_convertible<remove_diff_t<T>, ior::ior<T>>();
      py::implicitly_convertible<ior::reflectance<T>, ior::ior<T>>();
      py::implicitly_convertible<remove_diff_t<T>, ior::reflectance<T>>();
      py::implicitly_convertible<ior::ior<T>, ior::reflectance<T>>();

    }

    /********************************************************************/
    /*! \brief Define a python interface for ior::complex given a value type T.

      \tparam T = value type for ior::complex

    *********************************************************************/
    template<typename T>
      void complex_fresnel(py::module& m, const std::string& prefix = "")
    {
      py::class_<ior::complex_ior<T>>(m, (prefix + "ComplexIor").c_str())
        .def(py::init<const remove_diff_t<T>&>())
        .def(py::init<const remove_diff_t<T>&, const remove_diff_t<T>&>())
        .def(py::init<const ior::ior<T>&>())
        .def(py::init<const vec2d<T>&>())
        .def("__str__", [](const ior::complex_ior<T>& ior) { return bbm::toString(ior); });

      py::implicitly_convertible<ior::ior<T>, ior::complex_ior<T>>();
      py::implicitly_convertible<remove_diff_t<T>, ior::complex_ior<T>>();
      py::implicitly_convertible<vec2d<T>, ior::complex_ior<T>>();
    }
      
  } // end python namespace
} // end bbm namespace

#endif /* _BBM_PYTHON_FRESNEL_H_ */
