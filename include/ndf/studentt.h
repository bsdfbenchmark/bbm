#ifndef _BBM_STUDENT_T_NDF_H_
#define _BBM_STUDENT_T_NDF_H_

#include "bbm/ndf.h"

/************************************************************************/
/*! \file std.h

  \brief Implements the STD (Student's T-Distribution) Microfacet Normal
  Distribution from: "STD: Student’s t-Distribution of Slopes for Microfacet
  Based BSDFs" [Ribardiere et al. 2017] https://doi.org/10.1111/cgf.13137

*************************************************************************/

namespace bbm {
  namespace ndf {

    /********************************************************************/
    /*! /brief The StudentT Microfacet distribution

      \tparam CONF = bbm configuration
      \tparam Symmetry = isotropic or anisotropic (Default = symmetry_v::Isoptric)
      \tparam NAME = ndf name (default = STDist)

      Implements: concepts::ndf
    ********************************************************************/
    template<typename CONF, symmetry_v Symmetry=symmetry_v::Isotropic, string_literal NAME="StudentT"> requires concepts::config<CONF>
    class studentt
  {
  public:
    BBM_IMPORT_CONFIG( CONF );
    static constexpr string_literal name = NAME;

    /********************************************************************/
    /*! \brief Evaluate the NDF

      \param halfway = vector to eval the NDF for
      \param mask = enable/disbale lanes [default = true]
      \returns the resulting evaluation of the NDF
    *********************************************************************/
    Value eval(const Vec3d& halfway, Mask mask=true) const
    {
      // above surface?
      mask &= (vec::z(halfway) > 0);
      
      // Quick exit
      if(bbm::none(mask)) return 0;

      // eval NDF
      Vec2d alpha(roughness);
      Value alpha2 = bbm::hprod(alpha);
      Value normalization = Constants::Pi() * alpha2 * bbm::pow(spherical::cosTheta(halfway), 4);
      Value denumerator = bbm::pow(1.0 + bbm::squared_norm(vec::xy(halfway)/alpha) / ((gamma-1)*bbm::pow(vec::z(halfway), 2)), gamma);
      
      // Done.
      return bbm::select(mask, bbm::rcp(normalization * denumerator), 0);
    }

    /********************************************************************/
    /*! \brief Sample the NDF

      \param view = view direction (ignored)
      \param xi = 2D uniform random variables in [0..1] range
      \param Mask = enable/disbale lanes
      \returns A sampled microfacet normal.
     ********************************************************************/
    Vec3d sample(const Vec3d& /*view*/, const Vec2d& xi, Mask mask=true) const
    {
      // check valid xi
      mask &= (xi[0] >= 0) && (xi[1] >= 0) && (xi[0] <= 1) && (xi[1] <= 1);

      // quick exit
      if(bbm::none(mask)) return 0;

      // Get sin(phi) and cos(phi) & normalization
      Vec2d csp = bbm::cossin( Constants::Pi(2) * xi[0] );
      Value normalization;
      
      if constexpr (is_set(Symmetry, symmetry_v::Anisotropic))   // strech if anisotropic
      {
        normalization = bbm::rcp(bbm::squared_norm(csp / roughness));
        csp = bbm::normalize(csp * roughness);
      }
      else normalization = roughness*roughness;

      // compute sin(theta) and cos(theta)
      Value tanTheta2 = (bbm::pow(xi[1], 1.0/(1.0-gamma)) - 1) * (gamma-1) * normalization;
      Value cosTheta = bbm::rsqrt(1.0 + tanTheta2);
      Value sinTheta = bbm::safe_sqrt(1.0 - cosTheta*cosTheta);

      // Done.
      return bbm::select(mask, vec::expand(csp*sinTheta, cosTheta), 0);
    }

    /********************************************************************/
    /*! \brief PDF of sampling the NDF

      \param view = view direction (ignored)
      \param m = sampled microfacet normal
      \param mask = enable/disable lanes [default = true]
      \returns the PDF of sampling 'm' using the sample method.
    *********************************************************************/
    Value pdf(const Vec3d& /*view*/, const Vec3d& m, Mask mask=true) const
    {
      // m above surface?
      mask &= (vec::z(m) > 0);
      
      // quick bail out
      if(bbm::none(mask)) return 0;
      
      // eval PDF: D(m) |m.n|
      Value pdf = eval(m, mask) * spherical::cosTheta(m);

      // ignore negative values
      mask &= (pdf > 0);
      
      // Done.
      return select(mask, pdf, 0);
    }

    /********************************************************************/
    /*! \brief Monodirectional shadowing and masking term

      \param v = incident/outgoing vector
      \param m = microfacet normal
      \\param mask = enable/disable lanes
      \returns the monodirectional shadowing and masking attentuation factor

      There does not exist a closed form solution; use the same polynomial
      approximation as for Beckmann, except with a different 'a' value.
      *******************************************************************/
    Value G1(const Vec3d& v, const Vec3d& m, Mask mask=true) const
    {
      // check (dot(v,m) / dot(v,n)) > 0.
      mask &= (vec::z(v) > 0) && (bbm::dot(v, m) > 0);

      // Quick exit
      if(bbm::none(mask)) return 0;

      // check for normal incidence
      auto normalMask = (vec::z(v) < 1.0-Constants::Epsilon());
      if(bbm::none(normalMask)) return 1.0;

      // compute z = mu/sigma (Unnumbered Eq. between Eq. 18 and Eq. 19)
      Vec2d alpha(roughness);
      Value z = vec::z(v) * bbm::rsqrt( bbm::squared_norm(vec::xy(v) * alpha) ); 
      
      // S1: (Eq. 14)
      Value S1 = bbm::pow( (gamma-1) + z*z, 3.0/2.0 - gamma ) / z;
      
      // S2: (Eq. 22 & 23) (approx of Eq. 15)
      Value S2 = F21(z) * (F22(gamma) + F23(gamma)*F24(z));
      
      // Lambda: (Eq. 13)
      Value S1_scale = bbm::pow(gamma-1, gamma) / (2*gamma - 3);
      Value lambda = bbm::select(normalMask, bbm::tgamma(gamma - 0.5) / bbm::tgamma(gamma) * Constants::InvSqrtPi() * (S1_scale*S1 + bbm::sqrt(gamma-1)*S2) - 0.5, 0.0);
      
      // G1: (Eq 7)
      return bbm::select(mask, 1.0 / (1.0 + lambda), 0);
    }

  private:
    //! \brief F21(z) (Eq. 23)
    static inline Value F21(const Value& z)
    {
      Value z2 = z*z;  Value z3 = z2*z;
      Value numerator = 1.066*z +  2.655*z2 + 4.892*z3;
      Value denumerator = 1.038 + 2.969*z + 4.305*z2 + 4.418*z3;
      return (numerator / denumerator);
    }

    //! \brief F22(gamma) (Eq. 23)
    static inline Value F22(const Value& gamma)
    {
      Value gamma2 = gamma*gamma;  Value gamma3 = gamma2*gamma;
      Value numerator = 14.402 - 27.145*gamma + 20.574*gamma2 - 2.745*gamma3;
      Value denumerator = -30.612 + 86.567*gamma - 84.341*gamma2 + 29.938*gamma3;
      return (numerator / denumerator);
    }
    
    //! \brief F23(gamma) (Eq. 23)
    static inline Value F23(const Value& gamma)
    {
      Value gamma2 = gamma*gamma;  Value gamma3 = gamma2*gamma;
      Value numerator = -129.404 + 324.987*gamma - 299.305*gamma2 + 93.268*gamma3;
      Value denumerator = -92.609 + 256.006*gamma - 245.663*gamma2 + 86.064*gamma3;
      return (numerator / denumerator);
    }
    
    //! \brief F24(z) (Eq. 23)
    static inline Value F24(const Value& z)
    {
      Value z2 = z*z;  Value z3 = z2*z;
      Value numerator = 6.537 + 6.074*z - 0.623*z2 + 5.223*z3;
      Value denumerator = 6.538 + 6.103*z - 3.218*z2 + 6.347*z3;
      return (numerator / denumerator);
    }
  public:
  
    ///////////////////////////////
    //! @{ \name Class Attributes
    ///////////////////////////////
    specular_roughness<symmetry_t<Symmetry,Value>> roughness;
    bsdf_parameter<Value, bsdf_attr::SpecularParameter, 2.0, 40.0, 1.5 + Constants::Epsilon()> gamma;
    
    BBM_ATTRIBUTES( roughness, gamma );
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(studentt) {}
  };

  BBM_CHECK_CONCEPT(concepts::ndf, studentt<config>);
    
  } // end ndf namespace
} // end bbm namespace
  
#endif /* _BBM_STUDENT_T_NDF_H_ */
