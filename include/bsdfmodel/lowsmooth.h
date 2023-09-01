#ifndef _BBM_LOW_SMOOTH_H_
#define _BBM_LOW_SMOOTH_H_

#include "bbm/bsdfmodel.h"

/************************************************************************/
/*! \file lowsmooth.h
  
  \brief The Low et al.'s smooth BRDF model from "BRDF models for accurate and
  efficient rendering of glossy surfaces" [Low 2012]:
  https://doi.org/10.1145/2077341.2077350

*************************************************************************/

namespace bbm {

  template<typename CONF, string_literal NAME="LowSmooth"> requires concepts::config<CONF>
    class lowsmooth
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

      // Projected In and Out vectors
      Vec2d Ip = vec::xy(in);
      Vec2d Op = vec::xy(out);

      // Dp^2 (eq. 10) & cosThetaD (eq. 14)
      Value Dp2 = bbm::squared_norm(Ip + Op);
      Value cosThetaD = bbm::safe_sqrt(1 - 0.25*bbm::squared_norm(Ip-Op));

      // Eval (G == 1)
      Value S = bbm::pow(1.0 + B*Dp2, -C);
      Value Q = fresnel::cook<Config>::eval(eta.value(), cosThetaD, mask);

      // Done (eq. 12).
      return bbm::select(mask, A*S*Q, 0);
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

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return sample;

      // ro2
      Value ro2 = spherical::sinTheta2(out);
      
      // normalization factor*Pi (eq 19):
      Value temp = 1.0 + (2*B*(1.0 + ro2)) + bbm::pow(B*(1-ro2), 2);
      temp = -bbm::log(2.0) + bbm::log(1 + B*(1-ro2) + bbm::safe_sqrt(temp));
      Value MdPi = B * bbm::rcp(temp);   // InvPi cancels out with Pi in E.

      // E (eq. 22)
      Value E = 2.0 * bbm::exp( xi[0] * B * bbm::rcp(MdPi) );

      // Sample in disc (eq. 20)
      Value ri = bbm::safe_sqrt( (E-2)*(E + 2*B*ro2) / (2*E*B) );

      // Sample in disc (eq. 21)
      Value ro = bbm::sqrt(ro2);
      Value scale = bbm::sqrt( (1.0 + B*bbm::pow(ri + ro, 2.0)) / (1.0 + B*bbm::pow(ri - ro, 2.0)) );
      Value phi_i = 2.0 * bbm::atan( bbm::tan(xi[1] * Constants::Pi()) * scale ) + spherical::phi(out);

      // Fill in
      sample.direction = bbm::select(mask, vec::expand( bbm::cossin(phi_i) * ri, bbm::safe_sqrt(1.0 - ri*ri) ), 0);
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

      // ro2 = sin(theta_o)^2
      Value ro2 = spherical::sinTheta2(out);
      
      // normalization factor (eq 19):
      Value temp = 1.0 + (2*B*(1.0 + ro2)) + bbm::pow(B*(1.0-ro2), 2);
      temp = -bbm::log(Value(2)) + bbm::log(1 + B*(1-ro2) + bbm::safe_sqrt(temp));
      Value Md = B * Constants::InvPi() * bbm::rcp(temp);

      // PDF (eq 18)
      Value pdf = Md / (1.0 + B*bbm::squared_norm(vec::xy(in) + vec::xy(out)));

      // Done (eq. 17)
      return bbm::select(mask, pdf * spherical::cosTheta(in), 0);   
    }

    /*******************************************************************/
    /*! \brief Return the (approximate) hemispherical reflectance of the BSDF

      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes
      \returns the approximate hemsispherical reflectance of the BSDF for a given direction
    ********************************************************************/
    Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const 
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);

      // check 'out' lies above the horizon
      mask &= (vec::z(out) > 0);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return 0;      

      // Ignore 'out' (assume = [0,0,1]), then S = A / (1+B*sin(Theta)^2)^C
      // C==1 => log(B+1) / 2B
      // C!=1 => 1.0 - (B+1)^(1-C) / (2B(C-1))
      Value factor = bbm::select( bbm::abs(C-1) < Constants::Epsilon(), bbm::log(B + 1) / (2*B), (1.0 - bbm::pow(B+1, 1-C)) / (2*B*(C-1)));

      // Fresnel == Reflectance at normal incidence.
      Value R0 = bbm::pow((eta - 1) / (eta + 1), 2);

      // Done.
      return bbm::select(mask, Constants::Pi(2)*A*factor*R0, 0);
    }
    
    ////////////////////////////////
    //! @{ \name Class Attributes
    ////////////////////////////////
    bsdf_parameter<Spectrum, bsdf_attr::SpecularParameter> A;
    bsdf_parameter<Value, bsdf_attr::SpecularParameter> B;
    bsdf_parameter<Value, bsdf_attr::SpecularParameter> C;
    fresnel_parameter<ior::ior<Value>> eta;

    BBM_ATTRIBUTES(A, B, C, eta);
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(lowsmooth) {}
  };

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, lowsmooth<config>);

} // end bbm namespace

#endif /* _BBM_LOW_SMOOTH_H_ */

BBM_EXPORT_BSDFMODEL(bbm::lowsmooth)
