#ifndef _BBM_MICROFACET_H_
#define _BBM_MICROFACET_H_

#include "concepts/ndf.h"
#include "concepts/maskingshadowing.h"

#include "bbm/bsdfmodel.h"

/************************************************************************/
/*! \file microfacet.h

  \brief Microfacet BSDF model following "Microfacet Models for Refraction
  through Rough Surfaces" [Walter et al. 2007]:
  http://dx.doi.org/10.2312/EGWR/EGSR07/195-206

  This does not include implementation of Shadowin and Masking functions (G)
  or of the microfacet distributions (D).

  TODO: include BTDF
************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Predefined normalization factors

    + Unnormalized = 1.0
    + Walter = 4.0
    + Cook = 3.1415...
  ***********************************************************************/
  struct microfacet_n
  {
    static constexpr literal Unnormalized = 1.0;
    static constexpr literal Walter = 4.0;
    static constexpr literal Cook = constants<double>::Pi();
  };
  
  /**********************************************************************/
  /*! \brief General microfacet BRDF model.
    
    \tparam NDF = microfacet normal distribution (concepts::ndf && std::default_initializable)
    \tparam MaskingShadowing = masking and shadowing function (concepts::maskingshadowing)
    \tparam Fresnel = fresnel implementation (requires concepts::fresnel)
    \tparam NormalizationFactor = see microfacet_n
    \tparam NAME = model name

    NDF, MaskingShadowing, and Fresnel must also meet config::matching_config
    
    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename NDF,
           typename MaskingShadowing,
           typename Fresnel, 
           auto NormalizationFactor=microfacet_n::Unnormalized,
           string_literal NAME="microfacet"
           > requires concepts::matching_config<NDF, MaskingShadowing, Fresnel> && concepts::ndf<NDF> && concepts::maskingshadowing<MaskingShadowing> && std::default_initializable<NDF> && concepts::fresnel<Fresnel>
    struct microfacet
  {
    BBM_IMPORT_CONFIG( NDF );
    static constexpr string_literal name = NAME;
    BBM_BSDF_FORWARD;

    
    /********************************************************************/
    /*! \brief Evaluate the BSDF for a given in-out direction pair

      \param in = incident direction
      \param out = outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes (e.g., for Packet eval)
      \returns Evaluation of the BSDF per spectrum.
    *********************************************************************/
    Spectrum  eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);
      
      // above surface?
      mask &= ((vec::z(in) > 0.0f) && (vec::z(out) > 0.0f));
      
      // Quick exit if mask is all negative
      if(bbm::none(mask)) return Spectrum(0);
      
      // Halfway vector
      Vec3d h = halfway(in, out);
      Value inh = bbm::dot(in, h);
      Value outh = bbm::dot(out, h);
      
      // eval NDF
      auto D = ndf.eval(h, mask);
      
      // eval shadowing and masking
      auto G = MaskingShadowing::eval(ndf, in, out, h, mask);
      
      // compute Fresnel (take average of inh and outh to avoid non-reciprocal differences due to round-off errors).
      auto F = Fresnel::eval(eta, Value(0.5) * (inh + outh), mask);

      // Done.
      Spectrum result = D * G * F / NormalizationFactor / (vec::z(in) * vec::z(out));
      return bbm::select(mask, result, 0);
    }


    /********************************************************************/
    /*! \brief Sample the microfacet BSDF given a direction and two random variables.

      \param out = outgoing direction
      \param xi = two random variables stored in a Vec2d
      \param component = which reflectance component to sample
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes.
      \returns A bsdfSample containing the sampled direction and the corresponding pdf.
    *********************************************************************/
    BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      BsdfSample sample = {0,0,bsdf_flag::None};

      // specular?
      mask &= is_set(component, bsdf_flag::Specular);
      
      // check for valid xi
      mask &= (xi[0] >= 0) && (xi[1] >= 0) && (xi[0] <= 1) && (xi[1] <= 1);

      // check out lies above the horizon
      mask &= (vec::z(out) > 0);
      
      // Quick exit if mask all negative
      if(bbm::none(mask)) return sample;
      
      // sample microfacet normal
      Vec3d normal_m = ndf.sample(out, xi, mask);
      
      // sample direction == reflection about normal_m
      sample.direction = bbm::select(mask, reflect(out, normal_m), 0);
      sample.pdf = pdf(sample.direction, out, component, unit, mask);
      sample.flag = bbm::select(mask, BsdfFlag(bsdf_flag::Specular), BsdfFlag(bsdf_flag::None));
      
      // Done.
      return sample;
    }
    

    /********************************************************************/
    /*! \brief Compute the pdf given an in and out direction

      \param in = the incoming (sampled) direction
      \param out = the outgoing (given) direction
      \param component = which reflectance component was sampled
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes
      \returns the PDF that the incoming direction would be sampled given the outgoing direction.
    *******************************************************************/
    Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);
      
      // check if 'out' and 'in' lie above the horizon
      mask &= (vec::z(out) > 0) && (vec::z(in) > 0);
      
      // Quick exit if mask is all negative
      if(bbm::none(mask)) return 0;      

      // Halfway (must lie above the horizon)
      Vec3d h = halfway(in, out);
      h = bbm::select(vec::z(h) < 0, -h, h);
      
      // eval PDF:  pdf(h) / (4 out . h)
      auto pdf = ndf.pdf(out, h, mask) / (4.0 * bbm::abs(bbm::dot( out, h )) );

      // Compute PDF
      return bbm::select(mask, pdf, 0);
    }
    

    /********************************************************************/
    /*! \brief Return the (approximate) hemispherical reflectance of the BSDF
      
      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes
      \returns the approximate hemispherical reflectance of the BSDF for a given direction
    ******************************************************************/
    Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);

      // check 'out' lies above the horizon
      mask &= (vec::z(out) > 0);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return 0;      
      
      // Approximate as perfect mirror
      return Fresnel::eval(eta.value(), vec::z(out), mask) / NormalizationFactor * 4.0;
    }
    
    ///////////////////////////////////////////////////////
    //! @{ \name Class Attributes are copied from the NDF
    ///////////////////////////////////////////////////////
    NDF ndf;
    fresnel_parameter<typename std::decay_t<Fresnel>::parameter_type> eta;
    
    BBM_ATTRIBUTES( attributes(ndf), eta );
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(microfacet) {}
  };

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, microfacet<ndf<>, maskingshadowing<>, concepts::archetype::fresnel<>>);
  
} // end bbm namespace


#endif /* _BBM_MICROFACET_H_ */
