#ifndef _BBM_WARD_H_
#define _BBM_WARD_H_

#include "bbm/bsdfmodel.h"

/************************************************************************/
/*! \file ward.h
  
  \brief Implements: "Measuring and modeling anisotropic reflection" [Ward
  1992]: https://doi.org/10.1145/142920.134078 following the notes from
  Bruce Walter: https://www.graphics.cornell.edu/~bjw/wardnotes.pdf

*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief The anisotropic Ward BSDF model.

    \tparam CONF = bbm configuration
    \tparam Symmetry = isotropic or anisotropic (default: symmetry_v::anisotropic)
    \tparam NAME = name of the BSDF model (Default: 'Ward')

    Implements: concepts::bsdfmodel
   **********************************************************************/
  template<typename CONF, symmetry_v Symmetry = symmetry_v::Anisotropic, string_literal NAME="Ward"> requires concepts::config<CONF>
    class ward
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

      // Evaluate BRDF
      Vec2d r(roughness);    // copy anisotropic | map isotropic to Vec2d.
      Vec3d H = in + out;

      Value normalizationFactor = Constants::Pi(4) * bbm::sqrt( vec::z(in) * vec::z(out) ) * vec::x(r) * vec::y(r);
      Value exponent = bbm::squared_norm(vec::xy(H) / r) / bbm::pow(vec::z(H), 2);

      Spectrum result = albedo;
      result *= bbm::exp(-exponent) / normalizationFactor;

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
      mask &=  is_set(component, bsdf_flag::Specular);
      
      // random variable in [0...1]?
      mask &= (xi[0] >= 0) && (xi[1] >= 0) && (xi[0] <= 1) && (xi[1] <= 1);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return sample;

      // Compute sin and cos of halfway::phi
      Vec2d r(roughness);
      Vec2d csp = bbm::normalize(bbm::cossin(Constants::Pi(2) * xi[0]) * r);

      // Compute sin and cos of halfway::theta 
      Value cosTheta = bbm::rsqrt( 1.0 - (bbm::log(xi[1]) / bbm::squared_norm(csp / r)) );
      Value sinTheta = bbm::safe_sqrt(1.0 - cosTheta*cosTheta);
      
      // Convert from halfway to the canonical domain
      Vec3d halfway = vec::expand(csp*sinTheta, cosTheta);
      sample.direction = bbm::select(mask, reflect(out, halfway), 0);
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
    ***********************************************************************/
    Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);

      // above surface?
      mask &= (vec::z(in) >= 0) && (vec::z(out) >= 0);

      // Quick bailout
      if(bbm::none(mask)) return 0;

      // compute PDF:
      Vec2d r(roughness);
      Vec3d h = halfway(in, out);

      Value normalizationFactor = Constants::Pi(4) * vec::x(r) * vec::y(r) * bbm::dot(in, h) * bbm::pow( spherical::cosTheta(h), 3);
      Value exponent = bbm::squared_norm(vec::xy(h) / r) / bbm::pow(vec::z(h), 2);
      Value pdf = bbm::exp(-exponent) / normalizationFactor;

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
    specular_roughness<symmetry_t<Symmetry, Value>> roughness;

    BBM_ATTRIBUTES(albedo, roughness);
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(ward) {}
  };

  BBM_CHECK_CONCEPT( concepts::bsdfmodel, ward<config>);
  
} // end bbm namespace

#endif /* _BBM_WARD_H_ */

BBM_EXPORT_BSDFMODEL(bbm::ward)
