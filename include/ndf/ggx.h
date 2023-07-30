#ifndef _BBM_GGX_NDF_H_
#define _BBM_GGX_NDF_H_

#include "bbm/ndf.h"

/************************************************************************/
/*! \file ggx.h

  \brief Implements the GGX Microfacet Normal Distribution from: "Microfacet
  Models for Refraction through Rough Surfaces" [Walter et al. 2007]:
  http://dx.doi.org/10.2312/EGWR/EGSR07/195-206

  Anisotropic GGX distribution follows: "Understanding the Masking-Shadowing
  Function in Microfacet-Based BRDFs" [Heitz 2014]:
  https://jcgt.org/published/0003/02/03/
  
  Following the importance sampling of visible normals from: "A Simpler and
  Exact Sampling Routine for the GGX Distribution of Visible Normals" [Heitz
  2017]: https://jcgt.org/published/0007/04/01/
  
***********************************************************************/
  
namespace bbm {
  namespace ndf {
  
  /**********************************************************************/
  /*! \brief The GGX Microfacet distribution

    \tparam CONF = bbm configuration
    \tpatam Symmetry = symmetry_v::Isotropic or symmetry_v::Anisotropic (Default = symmetry_v::Isotropic)
    \tparam SampleVisible = sample visible normal distribution (Default = true)
    \tparam NAME = ndf name (default = GGX)

    Implements: concepts::ndf
  ************************************************************************/
  template<typename CONF, symmetry_v Symmetry=symmetry_v::Isotropic, bool SampleVisible=true, string_literal NAME="GGX"> requires concepts::config<CONF>
    class ggx
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
      Value D = bbm::rcp( Constants::Pi() * alpha2 * bbm::pow( bbm::squared_norm(vec::xy(halfway) / alpha) + bbm::pow(vec::z(halfway), 2), 2.0 ) );
      
      // Done.
      return bbm::select(mask, D, 0);
    }

    /********************************************************************/
    /*! \brief Sample the NDF

      \param view = view direction
      \param xi = 2D uniform random variables in [0..1] range
      \param Mask = enable/disbale lanes
      \returns A sampled microfacet normal.
     ********************************************************************/
    Vec3d sample(const Vec3d& view, const Vec2d& xi, Mask mask=true) const
    {
      // check valid xi
      mask &= (xi[0] >= 0) && (xi[1] >= 0) && (xi[0] <= 1) && (xi[1] <= 1);

      // quick exit
      if(bbm::none(mask)) return 0;

      // sample microfacet normal
      if constexpr (SampleVisible)           // Following [Heitz 2017]
      {
        Vec2d alpha(roughness);
        
        // 1) Stretch
        Vec3d view_s = bbm::normalize( vec::expand(vec::xy(view)*alpha, vec::z(view)) );

        // 2) Orthonormal basis
        Vec3d T1 = bbm::select(vec::z(view_s) < 1.0-Constants::Epsilon(), bbm::normalize(bbm::cross(view_s, Vec3d(0,0,1))), Vec3d(1,0,0));
        Vec3d T2 = bbm::cross(T1, view_s);

        // 3) sample polar coordinates
        Value a = bbm::rcp(1.0 + vec::z(view_s));
        Value r = bbm::sqrt(xi[0]);
        Value phi = bbm::select(xi[1] < a, xi[1]/a,  1.0 + (xi[1]-a) / (1.0-a)) * Constants::Pi();
        Vec2d csp = bbm::cossin(phi);
        Value P1 = r*csp[0];
        Value P2 = bbm::select(xi[1] < a, 1.0, vec::z(view_s)) * r * csp[1];

        // 4) normal
        Vec3d normal = P1*T1 + P2*T2 + bbm::safe_sqrt(1.0 - P1*P1 - P2*P2)*view_s;

        // 5) unstretch
        return bbm::normalize( vec::expand(vec::xy(normal) * alpha, bbm::max(0.0, vec::z(normal))) );
      }
      
      else // if constexpr (!SampleVisible)  // Following [Walter 2007] (w/ anisoptric extension)
      {
        // 1) get sin(phi) an cos(phi) & normalization
        Vec2d csp = bbm::cossin( Constants::Pi(2) * xi[0] );
        Value normalization;
        
        if constexpr (is_set(Symmetry, symmetry_v::Anisotropic))  // stretch if anisotropic
        {
          csp *= roughness;
          normalization = bbm::squared_norm(csp);
          csp *= bbm::rsqrt(normalization);  // normalize sin & cos
        }
        else normalization = roughness*roughness;                 // isotropic normalization
        
        // 2) compute sin(theta) and cos(theta)
        Value tanTheta2 = normalization * (xi[1] / (1-xi[1]));
        Value cosTheta = bbm::rsqrt(1.0 + tanTheta2);
        Value sinTheta = bbm::safe_sqrt(1.0 - cosTheta*cosTheta);

        // Done.
        return bbm::select(mask, vec::expand(csp*sinTheta, cosTheta), 0);
      }
    }

    /********************************************************************/
    /*! \brief PDF of sampling the NDF

      \param view = view direction (ignored)
      \param m = sampled microfacet normal
      \param mask = enable/disable lanes [default = true]
      \returns the PDF of sampling 'm' using the sample method.
    *********************************************************************/
    Value pdf(const Vec3d& view, const Vec3d& m, Mask mask=true) const
    {
      // m above surface?
      mask &= (vec::z(m) > 0);
      
      // quick bail out
      if(bbm::none(mask)) return 0;
      
      // eval PDF:
      Value pdf = eval(m, mask);

      if constexpr (SampleVisible) 
        pdf *= G1(view, m, mask) * bbm::abs(bbm::dot(view, m)) / spherical::cosTheta(view);
      else // if constexpr (!SampleVisible)
        pdf *= spherical::cosTheta(m);
        
      // ignore negative values (round off errors)
      mask &= (pdf > 0);

      // Done.
      return bbm::select(mask, pdf, 0);
    }

    /********************************************************************/
    /*! \brief Monodirectional shadowing and masking term

      \param v = incident/outgoing vector
      \param m = microfacet normal
      \param mask = enable/disable lanes
      \returns the monodirectional shadowing and masking attentuation factor
      *******************************************************************/
    Value G1(const Vec3d& v, const Vec3d& m, Mask mask=true) const
    {
      // check (dot(v,m) / dot(v,n)) > 0.
      mask &= (vec::z(v) > 0) && (bbm::dot(v, m) > 0);
       
      // Quick exit
      if(bbm::none(mask)) return 0;
      
      // eval
      Vec2d r(roughness);
      Value roughness2 = bbm::hprod(r);
      Value tanTheta2 = spherical::tanTheta2(v);
      Value denom = 1.0 + bbm::sqrt(1.0 + roughness2*tanTheta2);

      // Done.
      return bbm::select(mask, 2.0 / denom, 0.0);
    }
  
    ///////////////////////////////
    //! @{ \name Class Attributes
    ///////////////////////////////
    specular_roughness<symmetry_t<Symmetry,Value>> roughness;

    BBM_ATTRIBUTES( roughness );
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(ggx) {}
  };

  BBM_CHECK_CONCEPT(concepts::ndf, ggx<config>);

  } // end ndf namespace
} // end bbm namespace

#endif /* _BBM_GGX_NDF_H_ */
