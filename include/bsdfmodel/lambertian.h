#ifndef _BBM_LAMBERTIAN_H_
#define _BBM_LAMBERTIAN_H_

#include "bbm/bsdfmodel.h"

/**********************************************************************/
/*! \file lambertian.h
    \brief The classic diffuse Lambertian BSDF model.
***********************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief The classic diffuse Lambertian BSDF model.

    \tparam CONF = type configuration to use.
    \tparam NAME = name of the BSDF model.

    
    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, string_literal NAME="Lambertian"> requires concepts::config<CONF>
    struct lambertian
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

      Note: Evaluates the classic diffuse BSDF, i.e.. a constant independent
      of in and out directions above the horizon. No foreshortning is
      included.
    *********************************************************************/
    Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // diffuse?
      mask &= is_set(component, bsdf_flag::Diffuse);
        
      // above surface?
      mask &= (vec::z(in) >= 0) && (vec::z(out) >= 0);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return Spectrum(0);
        
      // compute reflectance
      Spectrum result = albedo * Constants::InvPi();
      return bbm::select(mask, result, 0);
    }


    
    /********************************************************************/
    /*! \brief Sample the diffuse BSDF given a direction and two random variables.

      \param out = outgoing direction
      \param xi = two random variables stored in a Vec2d
      \param component = which reflectance component to sample
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes.
      \returns A bsdfSample containing the sampled direction and the corresponding pdf.
     
      Follows the convention of Veach; samples incoming directions
      proportional to the cosine weighted solid angle.
    *********************************************************************/
    BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      BsdfSample sample = {0,0,bsdf_flag::None};

      // diffuse?
      mask &= is_set(component, bsdf_flag::Diffuse);
      
      // check for valid xi
      mask &= (xi[0] >=  0) && (xi[1] >= 0) && (xi[0] <= 1) && (xi[1] <= 1);
      
      // Quick exit if mask all negative
      if(bbm::none(mask)) return sample;
      
      // sample proportional to cosine weighted solid angle
      Vec2d csp = bbm::cossin(xi[0] * Constants::Pi(2));
      Value sinTheta = bbm::safe_sqrt(1.0 - xi[1]);
      
      sample.direction = vec::expand( csp*sinTheta, bbm::safe_sqrt(xi[1]));
      
      // compute pdf
      sample.pdf = pdf(sample.direction, out, component, unit, mask);
      
      // set component
      sample.flag = bbm::select(mask, BsdfFlag(bsdf_flag::Diffuse), BsdfFlag(bsdf_flag::None));
      
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
      // diffuse?
      mask &= is_set(component, bsdf_flag::Diffuse);
        
      // above surface?
      mask &= (vec::z(in) >= 0) && (vec::z(out) >= 0);

      // compute PDF (~ cosine weighted solid angle)
      return bbm::select(mask, vec::z(in) * Constants::InvPi(), 0);
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
      return bbm::select(mask & is_set(component, bsdf_flag::Diffuse), albedo, 0);
    }


    ///////////////////////////////
    //! @{ \name Class Attributes
    ///////////////////////////////
    diffuse_scale<Spectrum> albedo;
    
    BBM_ATTRIBUTES(albedo);
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(lambertian) {}
  };

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, lambertian<config>);
  
} // end bbm namespace

#endif /* _BBM_LAMBERTIAN_H_ */

BBM_EXPORT_BSDFMODEL(bbm::lambertian);


