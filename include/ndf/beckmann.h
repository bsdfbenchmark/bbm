#ifndef _BBM_BECKMANN_NDF_H_
#define _BBM_BECKMANN_NDF_H_

#include "bbm/ndf.h"

/************************************************************************/
/*! \file beckmann.h

  \brief Implements the Beckman Microfacet Normal Distribution:

  Beckmann, P. AND Spizzichino, A. "The Scattering of Electromagnetic Waves
  from Rough Surfaces". MacMillan, New York, 1963, pp. 1-33, 70-98.

  Following the description from: "Microfacet Models for Refraction through
  Rough Surfaces" [Walter et al. 2007]: http://dx.doi.org/10.2312/EGWR/EGSR07/195-206

  Following the importance sampling of visible norals from: "An Improved
  Visible Normal Sampling Routine for the Beckmann Distribution" [Jakob 2014]
  
***********************************************************************/
  
namespace bbm {
  namespace ndf {
  
  /**********************************************************************/
  /*! \brief The Beckman Microfacet distribution

    \tparam CONF = bbm configuration
    \tparam Symmetry = symmetry_v::Isotropic or symmetry_v::Anisotropic (Default = symmetry_v::Isotropic)
    \tparam Normalize = whether or not to normalize the NDF by 1/pi (Default = true)
    \tparam SampleVisible = sample visible normal distribution (Default = true)
    \tparam NAME = ndf name (default = Beckmann)

    Implements: concepts::ndf
  ************************************************************************/
    template<typename CONF, symmetry_v Symmetry=symmetry_v::Isotropic, bool Normalize=true, bool SampleVisible=true, string_literal NAME="Beckmann"> requires concepts::config<CONF>
    struct beckmann
  {
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
      Value cosTheta2 = spherical::cosTheta2(halfway);
      Value D = bbm::exp( -bbm::squared_norm(vec::xy(halfway) / alpha) / cosTheta2 ) / (vec::u(alpha) * vec::v(alpha) * cosTheta2 * cosTheta2);

      if constexpr (Normalize) D *= Constants::InvPi();
      
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
      if(bbm::none(mask)) return 0.0;

      // sample microfacet normal
      if constexpr (SampleVisible)  // Following [Jakob 2014]
      {
        Vec2d alpha(roughness);
        
        // 1) Stretch
        Vec3d view_s = bbm::normalize( vec::expand(vec::xy(view)*alpha, vec::z(view)) );

        // 2) sample P22
        Value tanTheta = spherical::tanTheta(view_s);

        Value maxval = bbm::erf(bbm::rcp(tanTheta));
        Vec2d xic = bbm::clamp(xi, 10e-6, 1.0-10e-6);
        Value x = maxval - (maxval + 1) * bbm::erf(bbm::sqrt(-bbm::log(xic[0])));

        xic[0] *= 1.0 + maxval + Constants::InvSqrtPi() * tanTheta * bbm::exp(-spherical::cosTheta2(view_s));

        for(size_t i=0; i < 3; ++i) // 3 Newton iterations
        {
          Value slope = bbm::erfinv(x);
          Value val = 1.0 + x + Constants::InvSqrtPi() * tanTheta * bbm::exp(-slope*slope) - xic[0];
          Value derivative = 1.0 - slope * tanTheta;
          x -= val / derivative;
        }

        Vec2d slope = bbm::select(x > -1.0 && x < +1.0, bbm::erfinv(Vec2d(x, 2.0*xic[1] - 1.0)), 0);

        // 3) rotate and unstretch
        Vec2d slope_u = rotation2d(spherical::cossinPhi(view_s)) * slope * alpha;

        // Done.
        return bbm::select(mask, bbm::normalize( vec::expand(-slope_u, 1) ), 0.0);
      }

      else // if constexpr (!SampleVisible)   // Following [Walter 2007] (w/ anisotropic extension)
      {
        // 1) get sin(phi) and cos(phi) & normalization
        Vec2d csp = bbm::cossin( Constants::Pi(2) * xi[0] );
        Value normalization;
        
        if constexpr (is_set(Symmetry, symmetry_v::Anisotropic))  // stretch if anisotropic
        {
          csp *= roughness;
          normalization = bbm::squared_norm(csp);
          csp *= bbm::rsqrt(normalization);
        }
        else normalization = roughness*roughness;                 // isotropic normalization
            
        // 2) compute sin(theta) and cos(theta)
        Value cosTheta = bbm::rsqrt(1.0 - normalization*bbm::log(xi[1]));
        Value sinTheta = bbm::safe_sqrt(1.0 - cosTheta*cosTheta);

        // Done.
        return bbm::select(mask, vec::expand(csp*sinTheta, cosTheta), 0.0);
      }
    }

    /********************************************************************/
    /*! \brief PDF of sampling the NDF

      \param view = view direction
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
      
      // eval PDF
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
      \\param mask = enable/disable lanes
      \returns the monodirectional shadowing and masking attentuation factor
      *******************************************************************/
    Value G1(const Vec3d& v, const Vec3d& m, Mask mask=true) const
    {
      // check (dot(v,m) / dot(v,n)) > 0.
      mask &= (vec::z(v) > 0) && (bbm::dot(v, m) > 0);
       
      // Quick exit
      if(bbm::none(mask)) return 0;
      
      // compute 'a'
      Value a;
      if constexpr (is_set(Symmetry, symmetry_v::Anisotropic))
      {
        a = bbm::rsqrt( bbm::squared_norm(vec::xy(v)*roughness) / bbm::pow(vec::z(v), 2) );
      }
      else a = bbm::rcp(roughness * spherical::tanTheta(v));

      // eval
      Value g = bbm::select (a < 1.6, (3.535*a + 2.181*a*a) / (1 + 2.276*a + 2.577*a*a), 1.0); 

      // rational approximation
      return bbm::select(mask, g, 0.0);
    }
  
    ///////////////////////////////
    //! @{ \name Class Attributes
    ///////////////////////////////
    specular_roughness<symmetry_t<Symmetry,Value>> roughness;

    BBM_ATTRIBUTES( roughness );
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(beckmann) {}
  };

  BBM_CHECK_CONCEPT(concepts::ndf, beckmann<config>);

  } // end ndf namespace
} // end bbm namespace

#endif /* _BBM_BECKMANN_NDF_H_ */
