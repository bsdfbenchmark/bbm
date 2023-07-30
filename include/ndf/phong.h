#ifndef _BBM_PHONG_NDF_H_
#define _BBM_PHONG_NDF_H_

#include "bbm/ndf.h"

/************************************************************************/
/*! \file phong.h

  \brief Implements the Phong Microfacet Normal Distribution from: "Microfacet
  Models for Refraction through Rough Surfaces" [Walter et al. 2007]:
  http://dx.doi.org/10.2312/EGWR/EGSR07/195-206

***********************************************************************/
  
namespace bbm {
  namespace ndf {
  
  /**********************************************************************/
  /*! \brief The Phong Microfacet distribution

    \tparam CONF = bbm configuration
    \tparam NAME = ndf name (default = Phong)

    Implements: concepts::ndf
  ************************************************************************/
  template<typename CONF, string_literal NAME="Phong"> requires concepts::config<CONF>
    class phong
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
      Value normalization = (sharpness + 2) / Constants::Pi(2);
      Value D = bbm::pow( spherical::cosTheta(halfway), sharpness ) * normalization;

      // Done.
      return bbm::select(mask, D, 0);
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

      // sample microfacet normal
      Value cosTheta = bbm::pow( xi[0], 1.0 / (sharpness + 2) );
      Value sinTheta = bbm::safe_sqrt(1.0 - cosTheta*cosTheta);
      Vec2d csp = bbm::cossin( xi[1] * Constants::Pi(2) );

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
      Value pdf = eval(m, mask) * bbm::abs( vec::z(m) );

      // Done.
      return bbm::select(mask, pdf, 0);
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
      
      // compute 'a'
      Value a = bbm::sqrt( 0.5*sharpness +1 ) / spherical::tanTheta(v);

      // eval
      Value g = bbm::select(a < 1.6, (3.535*a + 2.181*a*a) / (1 + 2.276*a + 2.577*a*a), 1.0); 
      
      // rational approximation
      return bbm::select(mask, g, 0.0);
    }
  
    ///////////////////////////////
    //! @{ \name Class Attributes
    ///////////////////////////////
    specular_sharpness<Value> sharpness;

    BBM_ATTRIBUTES( sharpness );
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(phong) {}
  };

  BBM_CHECK_CONCEPT(concepts::ndf, phong<config>);

  } // end ndf namespace
} // end bbm namespace

#endif /* _BBM_PHONG_NDF_H_ */
