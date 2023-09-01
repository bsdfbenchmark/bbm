#ifndef _BBM_PHONG_H_
#define _BBM_PHONG_H_

#include "bbm/bsdfmodel.h"

/************************************************************************/
/*! \file phong.h

  \brief Implements: "The Modifed Phong BSDF model for physically based rendering"
  [Lafortune 1994]: http://www.lafortune.eu/publications/Phong.html

*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief The modified Phong BSDF model. This is the classic Phong BSDF
    model with appropriate normalization and sampling methods.

    \tparam CONF = bbm configuration
    \tparam NAME = name of the BSDF model (Default: 'Phong')
    
    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, string_literal NAME="Phong"> requires concepts::config<CONF>
    class phong
  {
  public:
    BBM_IMPORT_CONFIG( CONF );
    static constexpr string_literal name = NAME;
    BBM_BSDF_FORWARD;
    
    /********************************************************************/
    /*! \brief Evaluate the BSDF for a given in and out direction

      \param in = incident direction
      \param out = outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes (e.g., for Packet eval)
      \returns Evaluation of the BSDF per spectrum.
    *********************************************************************/
    Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);
      
      // above surface?
      mask &= (vec::z(in) >= 0) && (vec::z(out) >= 0);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return Spectrum(0);

      // cos(Alpha) between reflected and outgoing direction
      Value cosAlpha = bbm::max(bbm::dot(reflect(in), out), 0);
      
      // Evaluate BSDF
      auto result = albedo;
      result *= (sharpness + 2) * Constants::InvPi(0.5) * bbm::pow(cosAlpha, sharpness);

      // Done.
      return bbm::select(mask, result, 0);
    }


    /********************************************************************/
    /*! \brief Sample the BSDF given a direction and two random variables.

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
      mask &= (xi[0] >=  0) && (xi[1] >= 0) && (xi[0] <= 1) && (xi[1] <= 1);
      
      // Quick exit if mask all negative
      if(bbm::none(mask)) return sample;
      
      // sample proportional to pow(cos(alpha), sharpness)
      Vec2d csp = bbm::cossin(xi[0] * Constants::Pi(2));
      Value cosTheta = bbm::pow(xi[1], 1.0 / (sharpness + 1));
      Value sinTheta = bbm::safe_sqrt( 1.0 - cosTheta*cosTheta );
      
      Vec3d in = vec::expand(csp * sinTheta, cosTheta);
      sample.direction = bbm::select(mask, toGlobalShadingFrame(reflect(out)) * in, 0);
      
      // compute pdf
      sample.pdf = pdf(sample.direction, out, component, unit, mask);
      
      // set component
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
    ***********************************************************************/
    Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);
        
      // above surface?
      mask &= (vec::z(in) >= 0) && (vec::z(out) >= 0);

      // Quick bailout
      if(bbm::none(mask)) return 0;
      
      // compute PDF: (sharpness+1)/2PI * pow(cos(alpha), sharpness)
      Value cosAlpha = bbm::max(bbm::dot(reflect(in), out), 0);
      Value pdf = (sharpness + 1) * Constants::InvPi(0.5) * bbm::pow(cosAlpha, sharpness);

      // Done.
      return bbm::select(mask, pdf, 0);
    }

    /*******************************************************************/
    /*! \brief Return the (approximate) hemispherical reflectance of the BSDF

      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes
      \returns the approximate hemispherical reflectance of the BSDF for a given direction
    ********************************************************************/
    Spectrum reflectance(const Vec3d& /*out*/, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      return bbm::select(mask & is_set(component, bsdf_flag::Specular), albedo, Spectrum(0));
    }

    
    ///////////////////////////////
    //! @{ \name Class Attributes
    ///////////////////////////////
    specular_scale<Spectrum> albedo;
    specular_sharpness<Value> sharpness;

    BBM_ATTRIBUTES(albedo, sharpness);
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(phong) {}
  };

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, phong<config>);
  
} // end bbm namespace

#endif /* _BBM_PHONG_H_ */

BBM_EXPORT_BSDFMODEL(bbm::phong)
