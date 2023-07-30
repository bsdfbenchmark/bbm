#ifndef _BBM_SCALED_MODEL_H_
#define _BBM_SCALED_MODEL_H_

#include "bbm/bsdfmodel.h"

/************************************************************************/
/*! \file scaledmodel.h

  \brief Add an 'albedo' parameter to an existing bsdfmodel and scale the
  albedo and eval respectively.

************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief Scaled BSDF model.

    \tparam BSDFMODEL = bsdf model to scale; must be default constructible.
    \tparam FLAG = bsdf_attr flag of the albedo attribute
    \tparam NAME = model name (default copy name from BSDFMODEL)

    Implements: concepts::bsdfmodel
  **********************************************************************/
  template<typename BSDFMODEL,
           bsdf_attr FLAG = bsdf_attr::Scale,
           string_literal NAME = BSDFMODEL::name
           > requires concepts::bsdfmodel<BSDFMODEL> && concepts::reflection::attributes<BSDFMODEL> && std::constructible_from<BSDFMODEL>
    struct scaledmodel : public BSDFMODEL
  {
    BBM_BASETYPES( BSDFMODEL );
    BBM_IMPORT_CONFIG( BSDFMODEL );
    static constexpr string_literal name = NAME;
    BBM_BSDF_FORWARD;
    
    // Copy sample and pdf unchanged from BSDFMODEL.
    using BSDFMODEL::sample;
    using BSDFMODEL::pdf;
    
    /********************************************************************/
    /*! \brief Evaluate the albedo*BSDFMODEL::eval

      \param in = incident direction
      \param out = outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes (e.g., for Packet eval)
      \returns Evaluation of the BSDF per spectrum.
    *********************************************************************/
    Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      return BSDFMODEL::eval(in, out, component, unit, mask) * albedo;
    }

    /*******************************************************************/
    /*! \brief Return albedo*BSSFMODEL::reflectance

      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes
      \returns the approximate hemispherical reflectance of the BSDF for a given direction
    ********************************************************************/
    Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      return BSDFMODEL::reflectance(out, component, unit, mask) * albedo;
    }

    
    //////////////////////////
    //! @{ Class Attributes
    //////////////////////////
    bsdf_scale<Spectrum,FLAG> albedo;

    BBM_ATTRIBUTES(albedo);
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(scaledmodel) {}
  };
    
} // end bbm namespace

#endif /* _BBM_SCALED_MODEL_H_ */
