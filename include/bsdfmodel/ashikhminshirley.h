#ifndef _BBM_ASHIKHMIN_SHIRLEY_H_ 
#define _BBM_ASHIKHMIN_SHIRLEY_H_

#include "bbm/bsdfmodel.h"

/************************************************************************/
/*! \file ashikhminshirley.h

  \brief Implements the specular component from: "An anisotropic phong BRDF
  model" [Ashikhmin and Shirley 2000]:
  https://doi.org/10.1080/10867651.2000.10487522

************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief the Anisotropic Phong BSSF model by Ashikhmin and Shirley.  This
      implements the specular component of the model.

      \tparam CONF = bbm configuration

      \tparam Fresnel = fresnel implementation (requires concepts::Fresnel); default = fresnel::schlick<Config, ior::reflectance<Spectrum>>
      \tparam Symmetry = pass symmetry_v (Default: symmetry_v::Anisotropic)
      \tparam NAME = name of the BSDF model (Default: 'AshikhminShirley')

      Implements: concepts::bsdfmodel
  **********************************************************************/
  template<typename CONF,
           typename Fresnel = fresnel::schlick<CONF, ior::reflectance<Spectrum_t<CONF>>>,
           symmetry_v Symmetry = symmetry_v::Anisotropic,
           string_literal NAME="AshikhminShirley"
           > requires  concepts::config<CONF> && concepts::fresnel<Fresnel> && concepts::matching_config<CONF, Fresnel>
    struct ashikhminshirley
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
      mask &= (vec::z(in) > 0) && (vec::z(out) > 0);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return Spectrum(0);

      // evaluate bsdf
      Vec3d h = halfway(in, out);
      Value h_dot_in = bbm::dot(h, in);
      Value denominator = h_dot_in * bbm::max( vec::z(in), vec::z(out) );
      Spectrum F = Fresnel::eval( fresnelReflectance, h_dot_in, mask );

      Value exponent, normalization;
      if constexpr (is_set(Symmetry, symmetry_v::Anisotropic))      // anisotropic
      {
        exponent = bbm::select( vec::z(h) < 1 - Constants::Epsilon(), ((vec::u(sharpness) * bbm::pow(vec::x(h), 2)) + (vec::v(sharpness) * bbm::pow(vec::y(h), 2))) / (1 - bbm::pow(vec::z(h), 2)), 0);
        normalization = bbm::sqrt( (vec::u(sharpness) + 1) * (vec::v(sharpness) + 1) ) / Constants::Pi(8);
      }
      else                                                          // isotropic
      {
        exponent = sharpness;
        normalization = (sharpness + 1) / Constants::Pi(8);
      }

      // Done.
      return bbm::select(mask, normalization * bbm::pow( vec::z(h), exponent ) * F / denominator, 0);
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

      // random variable in [0...1]?
      mask &= (xi[0] >= 0) && (xi[1] >= 0) && (xi[0] <= 1) && (xi[1] <= 1);

      // Quick exit if mask all negative
      if(bbm::none(mask)) return sample;

      // Sample halfway
      Vec2d csp;
      Value cosTheta;

      if constexpr (is_set(Symmetry, symmetry_v::Anisotropic))      // anisotrpic
      {
        // phi
        Value phi = bbm::atan( bbm::sqrt( (vec::u(sharpness) + 1.0) / (vec::v(sharpness) + 1.0) ) * bbm::tan(xi[0] * Constants::Pi(2)) );

        // make sure phi lies in same quadrant as 2PI xi[0]
        phi = bbm::select( (xi[0] > 0.25) && (xi[0] < 0.75), phi + Constants::Pi(), phi );

        // cosine theta
        csp = bbm::cossin(phi);
        cosTheta = bbm::pow(xi[1], 1.0 / ((vec::u(sharpness) * bbm::pow(vec::u(csp), 2)) + (vec::v(sharpness) * bbm::pow(vec::v(csp), 2)) + 1.0) );
      }
      else                                                          // isotropic
      {
        csp = bbm::cossin(xi[0] * Constants::Pi(2));
        cosTheta = bbm::pow(xi[1], 1.0 / (sharpness + 1.0));
      }

      Value sinTheta = bbm::safe_sqrt(1.0 - cosTheta*cosTheta);
      Vec3d h = vec::expand(csp * sinTheta, cosTheta);
      
      // Convert to canonical domain
      sample.direction = bbm::select(mask, reflect(out, h), 0);
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

      // Compute PDF
      auto h = halfway(in, out);
      auto h_dot_in = bbm::dot(h, in);

      Value exponent, normalization;
      if constexpr (is_set(Symmetry, symmetry_v::Anisotropic))      // anisotrpic
      {
        exponent = bbm::select( vec::z(h) < 1.0-Constants::Epsilon(), ((vec::u(sharpness) * bbm::pow(vec::x(h), 2.0)) + (vec::v(sharpness) * bbm::pow(vec::y(h), 2.0))) / (1.0 - bbm::pow(vec::z(h), 2)), 0);
        normalization = bbm::sqrt( (vec::u(sharpness) + 1) * (vec::v(sharpness) + 1) ) / Constants::Pi(2);
      }
      else                                                          // isotropic
      {
        exponent = sharpness;
        normalization = (sharpness + 1.0) / Constants::Pi(2);
      }

      auto pdf = normalization * bbm::pow(vec::z(h), exponent) / (4.0 * h_dot_in);

      // Done.
      return bbm::select(mask, pdf, 0);
    }

    
    /*******************************************************************/
    /*! \brief Return the (approximate) reflectance of the BSDF

      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes
      \returns the approximate reflectance of the BSDF for a given direction
    ********************************************************************/
    Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);

      // check 'out' lies above the horizon
      mask &= (vec::z(out) > 0);

      // Quick exit if mask all negative
      if(bbm::none(mask)) return 0;

      // Approximate as perfect mirror
      return Fresnel::eval( fresnelReflectance.value(), vec::z(out), mask );
    }

    /////////////////////////
    //! @{ Class Attributes
    /////////////////////////
    fresnel_parameter<typename Fresnel::parameter_type> fresnelReflectance;
    specular_sharpness<symmetry_t<Symmetry,Value>> sharpness;

    BBM_ATTRIBUTES(fresnelReflectance, sharpness);
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(ashikhminshirley) {}
  };

  BBM_CHECK_CONCEPT( concepts::bsdfmodel, ashikhminshirley<config> );

} // end bbm namespace


#endif /* _BBM_ASHIKHMIN_SHIRLEY_H_ */

BBM_EXPORT_BSDFMODEL(bbm::ashikhminshirley)
