#ifndef _BBM_PY_CORE_H_
#define _BBM_PY_CORE_H_

#include "pybind11/pybind11.h"
#include "pybind11/operators.h"

namespace py = pybind11;

#include "bbm/vec3dpair.h"
#include "bbm/bsdfsample.h"
#include "bbm/bsdf_ptr.h"
#include "bbm/aggregatebsdf.h"
#include "bbm/bsdf_enumerate.h"
#include "util/vector_util.h"

#include "python/py_fresnel.h"

/***********************************************************************/
/*! \file py_core.h
    \brief Export of core classes to python
************************************************************************/

namespace bbm {

  namespace python {

    /*******************************************************************/
    /*! \brief Define the Python interface for BBM core classes

      \tparam CONF = BBM configuration
      \param m = Python module

      Exports:
      + Ior
      + Reflectance
      + BsdfSample
      + BsdfPtr
      + AggregateBsdf
      + BsdfFlag

      All BSDFs are defined as a BsdfPtr in python.
    ********************************************************************/
    template<typename CONF>
      void core(py::module& m)
    {
      BBM_IMPORT_CONFIG( CONF );

      // Export bsdf_flag
      py::enum_<bsdf_flag>(m, "bsdf_flag")
        .value("None", bsdf_flag::None)
        .value("Diffuse", bsdf_flag::Diffuse)
        .value("Specular", bsdf_flag::Specular)
        .value("All", bsdf_flag::All)
        .def(py::self | py::self)
        .def(py::self & py::self)
        .def(py::self ^ py::self)
        .def(~py::self)
        .def("__str__", [](const bsdf_flag& flag) { return bbm::toString(flag); });

      // Export bsdf_attr flag
      py::enum_<bsdf_attr>(m, "bsdf_attr")
        .value("None", bsdf_attr::None)
        .value("DiffuseScale", bsdf_attr::DiffuseScale)
        .value("DiffuseParameter", bsdf_attr::DiffuseParameter)
        .value("SpecularScale", bsdf_attr::SpecularScale)
        .value("SpecularParameter", bsdf_attr::SpecularParameter)
        .value("Dependent", bsdf_attr::Dependent)
        .value("Diffuse", bsdf_attr::Diffuse)
        .value("Specular", bsdf_attr::Specular)
        .value("Scale", bsdf_attr::Scale)
        .value("Parameter", bsdf_attr::Parameter)
        .value("All", bsdf_attr::All)
        .def(py::self | py::self)
        .def(py::self & py::self)
        .def(py::self ^ py::self)
        .def(~py::self)
        .def("__str__", [](const bsdf_attr& flag) { return bbm::toString(flag); });
      
      // Export unit_t
      py::enum_<unit_t>(m, "unit_t")
        .value("Radiance", unit_t::Radiance)
        .value("Importance", unit_t::Importance)
        .def("__str__", [](const unit_t& unit) { return bbm::toString(unit); });

      // Export IOR
      fresnel<Value>(m);
      fresnel<Spectrum>(m, "Spectral");
      complex_fresnel<Value>(m);
      complex_fresnel<Spectrum>(m, "Spectral");
      
      // Export BsdfSample
      py::class_<BsdfSample>(m, "BsdfSample")
        .def_readonly("direction", &BsdfSample::direction, "Sampled direction")
        .def_readonly("pdf", &BsdfSample::pdf, "PDF of the sampled direction")
        .def_readonly("flag", &BsdfSample::flag, "Type of the sampled direction")
        .def("__str__", [](const BsdfSample& sample) { return bbm::toString(sample); });

      // Export Vec3dPair
      py::class_<Vec3dPair>(m, "Vec3dPair")
        .def_readonly("in", &Vec3dPair::in, "Incident direction")
        .def_readonly("out", &Vec3dPair::out, "Outgoing direction")
        .def("__str__", [](const Vec3dPair& sample) { return bbm::toString(sample); });

      // Export BsdfPtr
      py::class_<BsdfPtr>(m, "BsdfPtr")
        .def(py::init<const BsdfPtr &>())
        .def("eval", [](const BsdfPtr& self, const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) { return self.eval(in,out,component,unit,mask); }, "Given an 'in' and 'out' direction, return the radiance/importance of the BSDF", py::arg("in"), py::arg("out"), py::arg_v("component", bsdf_flag::All), py::arg_v("unit", unit_t::Radiance), py::arg_v("mask", true))
        .def("sample", [](const BsdfPtr& self, const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) { return self.sample(out, xi, component, unit, mask); }, "Given an 'in' direction, and 2D random variable ('xi'), generate a 'out' directions and corresponding PDF stored in a bsdfSample accordining to the radiance/importance distribution", py::arg("in"), py::arg("xi"), py::arg_v("component", bsdf_flag::All), py::arg_v("unit", unit_t::Radiance), py::arg_v("mask", true))
        .def("pdf", [](const BsdfPtr& self, const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) { return self.pdf(in, out, component, unit, mask); }, "Given an 'in' direction, produce the pdf of sampling the 'out' direction according to the radiance/importance distribution", py::arg("in"), py::arg("out"), py::arg_v("component", bsdf_flag::All), py::arg_v("unit", unit_t::Radiance), py::arg_v("mask", true))
        .def("reflectance", [](const BsdfPtr& self, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) { return self.reflectance(out, component, unit, mask); }, "Given an 'in' direction, return the approximate hemispherical reflectance (i.e., integral over all outgoing directions) of the radiance/importance of the BSDF", py::arg("in"), py::arg_v("component", bsdf_flag::All), py::arg_v("unit", unit_t::Radiance), py::arg_v("mask", true))
        .def("__str__", &BsdfPtr::toString, "BSDF attributes to string") 
        ;

      //! \brief Export aggregate BSDFs
      m.def("AggregateBsdf", [](py::args args)
      {
        bbm::vector<BsdfPtr&> arg_list;
        for(auto& a : args)
          arg_list.push_back( py::cast<BsdfPtr&>(*a) );

        return make_bsdf_ptr( aggregate( arg_list.begin(), arg_list.end() ) );
      }, "AggregateBsdf(BsdfPtr...) combines as many BsdfPtrs as provided.");

      /******************************************************************/
      /*! @{ \name Export parameter container
       ******************************************************************/
      py::class_<bbm::vector<Value&>>(m, "RefValueVector")
        .def(py::init<const bbm::vector<Value&>& >())
        .def("__len__", &bbm::vector<Value&>::size)
        .def("__setitem__", [](bbm::vector<Value&>& self, size_t index, const Value& val) { self[index] = val; })
        .def("__getitem__", [](bbm::vector<Value&>& self, size_t index) { return self[index]; })
        .def(py::self + py::self)
        .def(py::self + Value())
        .def(py::self += py::self)
        .def(py::self += Value())
        .def(py::self - py::self)
        .def(py::self - Value())
        .def(py::self -= py::self)
        .def(py::self -= Value())
        .def(py::self * py::self)
        .def(py::self * Value())
        .def(py::self *= py::self)
        .def(py::self *= Value())
        .def(py::self / py::self)
        .def(py::self / Value())
        .def(py::self /= py::self)
        .def(py::self /= Value())
        .def("__str__", [](const bbm::vector<Value&>& param) { return bbm::toString(param); })
        ;

      py::class_<bbm::vector<Value>>(m, "ValueVector")
        .def(py::init<const bbm::vector<Value>& >())
        .def("__len__", &bbm::vector<Value>::size)
        .def("__setitem__", [](bbm::vector<Value>& self, size_t index, const Value& val) { self[index] = val; })
        .def("__getitem__", [](bbm::vector<Value>& self, size_t index) { return self[index]; })
        .def(py::self + py::self)
        .def(py::self + Value())
        .def(py::self += py::self)
        .def(py::self += Value())
        .def(py::self - py::self)
        .def(py::self - Value())
        .def(py::self -= py::self)
        .def(py::self -= Value())
        .def(py::self * py::self)
        .def(py::self * Value())
        .def(py::self *= py::self)
        .def(py::self *= Value())
        .def(py::self / py::self)
        .def(py::self / Value())
        .def(py::self /= py::self)
        .def(py::self /= Value())
        .def("__str__", [](const bbm::vector<Value>& param) { return bbm::toString(param); })
        ;
      //! @}
      
      /******************************************************************/
      /*! @{ \brief Export methods for querying the parameters of a BSDF
       ******************************************************************/
      m.def("parameter_values", &parameter_values<BsdfPtr>, py::arg("bsdf"), py::arg_v("flag", bsdf_attr::All), "List all parameter values of a given BSDF");
      m.def("parameter_default_values", &parameter_default_values<BsdfPtr>, py::arg("bsdf"), py::arg_v("flag", bsdf_attr::All), "List all default parameter values of a given BSDF");
      m.def("parameter_lower_bound", &parameter_lower_bound<BsdfPtr>, py::arg("bsdf"), py::arg_v("flag", bsdf_attr::All), "List the lower bound of all parameter values of a given BSDF");
      m.def("parameter_upper_bound", &parameter_upper_bound<BsdfPtr>, py::arg("bsdf"), py::arg_v("flag", bsdf_attr::All), "List the upper vound of all parameter values of a given BSDF");
      //! @}
    }      

  } // end python namespace
} // end bbm namespace


#endif /* BBM_PY_CORE_H_ */
