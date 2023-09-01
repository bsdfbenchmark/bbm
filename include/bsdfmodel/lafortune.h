#ifndef _BBM_LAFORTUNE_H_
#define _BBM_LAFORTUNE_H_

#include "bbm/bsdfmodel.h"

/************************************************************************/
/*! \file lafortune.h

  \brief Implements: "Non-Linear Approximation of Reflectance Functions"
  [Lafortune 1997]: https://doi.org/10.1145/258734.258801

*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief The Lafortune BRDF model.  This implements the model in Eq. 4 in
      the original paper linked above:

      \f$ f_r(u, v) = \rho_s [ C_x u_x v_x + C_y u_y v_y + C_z u_z v_z ]^n \f$

     \tparam CONF = bbm configuration
     \tpatam Symmetry = isotropic or anisotropic (Default: symmetry_v::Anisotropic)
     \tparam NAME = name of the BSDF model (Default: 'Lafortune')

     Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, symmetry_v Symmetry = symmetry_v::Anisotropic, string_literal NAME="Lafortune"> requires concepts::config<CONF>
    class lafortune
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

      // Eval
      Vec2d cxy(Cxy);   // handle isotropic vs anisotropic
      Vec3d C = vec::expand(cxy, Cz);
      Value fr = bbm::pow( bbm::max(bbm::dot(C, in*out), 0), sharpness );
      
      // Done.
      return bbm::select(mask, albedo * fr, 0);
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
      
      Vec3d in = vec::expand(csp*sinTheta, cosTheta);
      
      // transform sampled direction to global frame
      Vec3d C = vec::expand(Vec2d(Cxy), Cz);
      sample.direction = toGlobalShadingFrame(bbm::normalize(C*out)) * in;
      
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
      Vec3d C = vec::expand(Vec2d(Cxy), Cz);
      Value cosAlpha = bbm::max(bbm::dot(bbm::normalize(C*out), in), 0);
      Value pdf = (sharpness + 1) / Constants::Pi(2) * bbm::pow(cosAlpha, sharpness);

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
    Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      Vec3d C = vec::expand(Vec2d(Cxy), Cz);
      Value normalization = bbm::pow( bbm::norm(C * out), sharpness ) * Constants::Pi(2) / (sharpness + 2);
      return bbm::select(mask & is_set(component, bsdf_flag::Specular), albedo * normalization, Spectrum(0));
    }

    
    //////////////////////////
    //! @{ Class Attributes
    //////////////////////////
    specular_scale<Spectrum> albedo;
    bsdf_parameter<symmetry_t<Symmetry,Value>, bsdf_attr::SpecularParameter, -0.57735026919, std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::min()> Cxy;
    bsdf_parameter<Value, bsdf_attr::SpecularParameter, 0.57735026919, std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::min()> Cz;
    specular_sharpness<Value> sharpness;

    BBM_ATTRIBUTES( albedo, Cxy, Cz, sharpness );
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(lafortune) {}
  };

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, lafortune<config>);

} // end bbm namespace
  
#endif /* _BBM_LAFORTUNE_H_ */

BBM_EXPORT_BSDFMODEL(bbm::lafortune)
