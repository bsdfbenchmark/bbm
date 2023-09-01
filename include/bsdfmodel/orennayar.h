#ifndef _BBM_ORENNAYAR_H_
#define _BBM_ORENNAYAR_H_

#include "bbm/bsdfmodel.h"
#include "bsdfmodel/lambertian.h"

/************************************************************************/
/*! \file orennayar.h

  \brief Implements: "Generalization of Lambert's Reflectance Model" [Oren and
  Nayar 1994]: https://doi.org/10.1145/192161.192213

*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief The Oren-Nayar BSDF microfacet model for diffuse surfaces. 

    \tparam CONF = bbm configuration
    \tpatam NAME = name of the BSDF model (Default: 'OrenNayar')

    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, string_literal NAME="OrenNayar"> requires concepts::config<CONF>
    class orennayar
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
      // diffuse?
      mask &= is_set(component, bsdf_flag::Diffuse);

      // above surface?
      mask &= (vec::z(in) > 0) && (vec::z(out) > 0);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return Spectrum(0);

      // Compute 'A' and 'B' parameters from roughness
      Value sigma2 = roughness * roughness;
      Value A = 1 - 0.5 * sigma2 / (sigma2 + 0.33);
      Value B = 0.45 * sigma2 / (sigma2 + 0.09);

      // eval: diffuse / PI * factor
      // factor = A + B * cos(phi(in) - phi(out)) * sin(alpha) * tan(beta)
      //
      // Simplify:
      // --------
      // cos(phi(in) - phi(out)) = cos(phi(in))*cos(phi(out)) - sin(phi(in))*sin(phi(out))
      //                         = (x(in) * x(out) - y(in)*y(out)) / (sin(theta(in)) * sin(theta(out)))
      //                         = x(in) * x(out) - y(in)*y(out) / (sinAlpha * sinBeta)
      //
      // Substitute in 'factor':
      // -----------------------
      // factor = A + B cos(phi(in) - phi(out)) * sinAlpha * tanBeta =
      //        = A + B * (x(in) * x(out) - y(in)*y(out) / (sinAlpha * sinBeta)) * (sinAlpha * sinBeta / cosBeta)
      //        = A + B * (x(in) * x(out) - y(in)*y(out)) / cosBeta
      //
      // with cosBeta = cos(min(theta(in), theta(out)))
      //              = max( cos(theta(in)), cos(theta(out)) )
      //
      auto cosBeta = bbm::max( spherical::cosTheta(in), spherical::cosTheta(out) );
      auto dot_xy = (vec::x(in) * vec::x(out)) + (vec::y(in) * vec::y(out));
      auto factor = A + (B * bbm::max(dot_xy, 0) / cosBeta);
      
      // compute reflectance
      Spectrum result = albedo / Constants::Pi() * factor;

      // Done.
      return bbm::select(mask, result, Spectrum(0));
    }

    
    /********************************************************************/
    /*! \brief Sample the BSDF given a direction and two random variables.

      \param out = outgoing direction
      \param xi = two random variables stored in a Vec2d
      \param component = which reflectance component to sample
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes.
      \returns A bsdfSample containing the sampled direction and the corresponding pdf.

      Approximate sampling by using the same sampling as Lambertian.
    *********************************************************************/
    BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      return lambertian<Config>().sample(out, xi, component, unit, mask);
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
    Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      return lambertian<Config>().pdf(in, out, component, unit, mask);
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
      return bbm::select(mask & is_set(component, bsdf_flag::Diffuse), albedo, Spectrum(0));
    }

    
    /////////////////////////
    //! @{ Class Attributes
    /////////////////////////
    diffuse_scale<Spectrum> albedo;
    diffuse_roughness<Value> roughness;

    BBM_ATTRIBUTES(albedo, roughness);
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(orennayar) {}
  };

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, orennayar<config>);
  
} // end bbm namespace
  
#endif /* BBM_ORENNAYAR_H_ */

BBM_EXPORT_BSDFMODEL(bbm::orennayar)
