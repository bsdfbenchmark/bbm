#ifndef _BBM_LOW_NDF_H_
#define _BBM_LOW_NDF_H_

#include "bbm/ndf.h"

/************************************************************************/
/*! \file low.h

  \brief Implements Low et al.'s Microfacet Normal Distribution from "BRDF
  models for accurate and efficient rendering of glossy surfaces" [Low 2012]:
  https://doi.org/10.1145/2077341.2077350

***********************************************************************/
  
namespace bbm {
  namespace ndf {
  
  /**********************************************************************/
  /*! \brief The Low Microfacet distribution

    \tparam CONF = bbm configuration
    \tparam NAME = ndf name (default = Low)

    The NDF is parameterized by two unnormalized parameters 'B' and 'C'.  The
    'A' scale parameter is not included.  Note: this is not a true NDF, as the
    eval function is not normalized!
    
    Implements: concepts::ndf
  ************************************************************************/
  template<typename CONF, string_literal NAME="Low"> requires concepts::config<CONF>
    class low
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
      Value S = bbm::pow(1.0 + B*(1.0 - vec::z(halfway)), -C);

      // Done.
      return bbm::select(mask, S, 0);
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
      if(bbm::none(mask)) return 0.0;

      // sample microfacet normal
      Value term = bbm::select( bbm::abs(C-1) < Constants::Epsilon(),
                                bbm::exp( xi[0] * bbm::log(1.0 + B) ),   // C ~ 1
                                bbm::pow(1.0 + xi[0] * (bbm::pow(1.0+B, 1.0-C) - 1.0), -1.0 / (C - 1.0))  // C != 1
                                );
      Value cosTheta = (1.0 + B - term) / B;
      Value sinTheta = bbm::safe_sqrt( 1.0 - cosTheta*cosTheta );
      Vec2d csp = bbm::cossin( xi[1] * Constants::Pi(2) );

      // Done.
      return bbm::select(mask, vec::expand(csp*sinTheta, cosTheta), 0.0);
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
      
      // eval PDF: D(m) * normalization
      Value normalization = bbm::select( bbm::abs(C-1) < Constants::Epsilon(),
                                         1.0f / bbm::log(1.0+B), // C ~ 1
                                         (C-1.0) / (1.0 - bbm::pow(1.0+B, 1.0-C)) // C != 1
                                         );
      Value pdf = eval(m, mask) * B * (Constants::InvPi(0.5) * normalization);

      
      // ignore negative values (round off errors)
      mask &= (pdf > 0);

      // Done.
      return bbm::select(mask, pdf, 0);
    }

    /********************************************************************/
    /*! \brief Monodirectional shadowing and masking factor

      \param v = incident/exitant direction of transport
      \param m = microfacet normal.
      \param mask = enable/disable lanes [default = true]

      Low et al. did not propose a shadowing and masking function based on
      the NDF, and used the v-groove model instead
    *********************************************************************/
    Value G1(const Vec3d& /*v*/, const Vec3d& /*m*/, Mask /*mask*/=true) const
    {
      return 1;
    }

    ///////////////////////////////
    //! @{ \name Class Attributes
    ///////////////////////////////
    bsdf_parameter<Value, bsdf_attr::SpecularParameter> B;
    bsdf_parameter<Value, bsdf_attr::SpecularParameter> C;

    BBM_ATTRIBUTES( B, C );
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(low) {}
  };

  BBM_CHECK_CONCEPT(concepts::ndf, low<config>);

  } // end ndf namespace
} // end bbm namespace

#endif /* _BBM_LOW_NDF_H_ */
