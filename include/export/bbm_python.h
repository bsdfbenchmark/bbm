/***********************************************************************/
/*! \file bbm_python.h
    \brief Python support for BBM

    \param bsdfmodel = the class name of the BSDF model to export. 

    Assumes that:
    1.  the bsdfmodelclass has a single (non-defaulted) template parameter to specify
    the Configuration.  
    2. the BSDFMDOEL has a static constexpr string_literal name.
    3. the BSDFMODEL should have a constructor with a named arg list and constructor_args_t is defined.

    The macro will then create a default trampoline function that constructs a
    bsdf_ptr based on the parameters passed via python (args and kwargs). The
    python args/kwargs are compared to the attributes and corresponding
    attributes are set.

    To compile the bsdf models in a python module, a py::module& named 'm'
    needs to be defined. This is the module the bsdf model is exported to.

    If met, simply re-include this file and the bsdfmodels you want 
    to include.  See bbm_python.cpp for an example.
  
************************************************************************/

#undef BBM_EXPORT_BSDFMODEL
#define BBM_EXPORT_BSDFMODEL(bsdfmodel)  bbm::python::def_bsdf<bsdfmodel<bbm::BBM_CONFIG>>(m);

