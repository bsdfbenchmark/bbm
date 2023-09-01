#ifndef _BBM_LOW_MICROFACET_H_
#define _BBM_LOW_MICROFACET_H_

#include "bbm/bsdfmodel.h"
#include "bsdfmodel/microfacet.h"
#include "ndf/low.h"
#include "maskingshadowing/vgroove.h"

/************************************************************************/
/*! \file lowmicrofacet.h

  \brief The Low et al.'s microfacet BRDF model from "BRDF models for accurate
  and efficient rendering of glossy surfaces" [Low 2012]:
  https://doi.org/10.1145/2077341.2077350

*************************************************************************/
namespace bbm {

  /**********************************************************************/
  /*! \brief The Low et al. Microfacet BRDF model.

    \tparam CONF = bbm configuration
    \tparam NormalizationFactor = see below (Default: normalization_v::Cook)
    \tparam NAME= model name (default = LowMicrofacet)

    The Low et al. Microfacet BSDF model is a specialization microfacet that uses
    an unnormalized scale 'A'.

    Both Low et al's supplemental document as well as the paper (Eq. 15) do
    not list any normalization factor, which makes sense since 'A' is not
    normalized.  This is the default behavior of this implementation. However,
    the MERL-MIT BRDF fits listed in Low et al.'s supplemental document
    includes an additional (incorrect) normalization factor of 'Pi'
    
    Implements: concepts::bsdfmodel
  ***********************************************************************/
    
  template<typename CONF, auto NormalizationFactor = microfacet_n::Cook, string_literal NAME = "LowMicrofacet"> requires concepts::config<CONF>
    class lowmicrofacet : public microfacet<ndf::low<CONF>,
                                            maskingshadowing::vgroove<CONF>,
                                            fresnel::cook<CONF>,
                                            NormalizationFactor,
                                            NAME>
  {
    using base = microfacet<ndf::low<CONF>, maskingshadowing::vgroove<CONF>, fresnel::cook<CONF>, NormalizationFactor, NAME>;
    BBM_BASETYPES(base);
  public:
    BBM_IMPORT_CONFIG( CONF );
    static constexpr string_literal name = NAME;

    // copy sample and pdf methods unchanged from microfacet
    using base::sample;
    using base::pdf;

    /********************************************************************/
    /*! \brief Evaluate the A*base::eval

      \param in = incident direction
      \param out = outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes (e.g., for Packet eval)
      \returns Evaluation of the BSDF per spectrum.
    *********************************************************************/
    Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      return base::eval(in, out, component, unit, mask) * A;
    }

    /*******************************************************************/
    /*! \brief Return albedo*base::reflectance

      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes
      \returns the approximate hemispherical reflectance of the BSDF for a given direction
    ********************************************************************/
    Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      return base::reflectance(out, component, unit, mask) * A;
    }

    
    /////////////////////////
    //! @{ Class Attributes
    /////////////////////////
    bsdf_parameter<Spectrum, bsdf_attr::SpecularParameter> A;

    BBM_ATTRIBUTES(A);
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(lowmicrofacet) {}
  };

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, lowmicrofacet<config>);
  
} // end bbm namespace

#endif /* _BBM_LOW_MICROFACET_H_ */

BBM_EXPORT_BSDFMODEL(bbm::lowmicrofacet)

