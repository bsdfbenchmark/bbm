#ifndef _BBM_NDF_EPD_H_
#define _BBM_NDF_EPD_H_

#include "bbm/ndf.h"
#include "util/gamma.h"
#include "util/invgamma.h"

#include "precomputed/holzschuchpacanowski/G1.h"
#include "precomputed/holzschuchpacanowski/distributionNormalization.h"

/************************************************************************/
/*! \file epd.h

  \brief The Exponential Power Distribution from "A two-scale microfacet
  reflectance model combining reflection and diffraction", Holzschuch and
  Pacanowski [2017]: https://doi.org/10.1145/3072959.3073621

  Implements the EPD Eq. 34-38 & 49-50

*************************************************************************/

namespace bbm {
  namespace ndf {

    /********************************************************************/
    /*! \brief EPD variants: compute the noralization on the fly or use precomputed values.
     ********************************************************************/
    enum struct epd_normalization
    {
      Compute,
      Precomputed
    };

    /********************************************************************/
    /*! \brief Exponential Power Distribution

      \tparam CONF = bbm configuration
      \tparam NORMALIZATION = use precomputed normalization of compute on the fly (default=epd_normalization::Compute)
      \tparam NAME = model name (default = "EPD")

      Implements: concepts::ndf
    *********************************************************************/
    template<typename CONF,
             epd_normalization NORMALIZATION=epd_normalization::Compute,
             string_literal NAME="EPD"
             > requires concepts::config<CONF>
      class epd
    {
    public:
      BBM_IMPORT_CONFIG( CONF );
      static constexpr string_literal name = NAME;

      /******************************************************************/
      /*! \brief Evaluate the NDF

        \param halfway = vector to eval the NDF for
        \param mask = enable/disbale lanes [default = true]
        \returns the resulting evaluation of the NDF
      *******************************************************************/
      Value eval(const Vec3d& halfway, Mask mask=true) const
      {
        // above surface?
        mask &= (vec::z(halfway) > 0);
      
        // Quick exit
        if(bbm::none(mask)) return 0;

        // eval NDF (eq. 34-35)
        Value cosTheta2 = spherical::cosTheta2(halfway);
        Value tanTheta2 = (1 - cosTheta2) / cosTheta2;
        Value beta2 = beta*beta;
        Value normalization = compute_normalization();
        Value D = normalization * bbm::exp( -bbm::pow( tanTheta2 / beta2, p ) ) / (cosTheta2*cosTheta2);

        return bbm::select(mask, D, 0);
      }


      /******************************************************************/
      /*! \brief Sample the NDF
        
        \param view = view direction (ignored)
        \param xi = 2D uniform random variables in [0..1] range
        \param Mask = enable/disbale lanes
        \returns A sampled microfacet normal.
      *******************************************************************/
      Vec3d sample(const Vec3d& /*view*/, const Vec2d& xi, Mask mask=true) const
      {
        // check valid xi
        mask &= (xi[0] >= 0) && (xi[1] >= 0) && (xi[0] <= 1) && (xi[1] <= 1);

        // quick exit
        if(bbm::none(mask)) return 0;

        // Sample (Eq. 49-50)
        Vec2d csp = bbm::cossin( Constants::Pi(2) * xi[0] );
        Value inv_p = bbm::rcp(p);
        Value tanTheta2 = beta * beta * bbm::pow( bbm::gamma_q_inv(inv_p, xi[1]), inv_p );
        Value cosTheta = bbm::rsqrt(1.0 + tanTheta2);
        Value sinTheta = bbm::safe_sqrt(1.0 - cosTheta*cosTheta);

        // Done.
        return bbm::select(mask, vec::expand(csp*sinTheta, cosTheta), 0.0);
      };

      
      /******************************************************************/
      /*! \brief PDF of sampling the NDF

        \param view = view direction (ignored)
        \param m = sampled microfacet normal
        \param mask = enable/disable lanes [default = true]
        \returns the PDF of sampling 'm' using the sample method.
      *******************************************************************/
      Value pdf(const Vec3d& /*view*/, const Vec3d& m, Mask mask=true) const
      {
        // m above surface?
        mask &= (vec::z(m) > 0);
        
        // quick bail out
        if(bbm::none(mask)) return 0;

        // eval PDF
        Value pdf = eval(m, mask) * spherical::cosTheta(m);

        // ignore negative values (round off errors)
        mask &= (pdf > 0);
        
        // Done.
        return bbm::select(mask, pdf, 0);
      }


      /******************************************************************/
      /*! \brief Monodirectional shadowing and masking term

        \param v = incident/outgoing vector
        \param m = microfacet normal
        \param mask = enable/disable lanes
        \returns the monodirectional shadowing and masking attentuation factor
        ****************************************************************/
      Value G1(const Vec3d& v, const Vec3d& m, Mask mask=true) const
      {
        // check (dot(v,m) / dot(v,n)) > 0.
        mask &= (vec::z(v) > 0) && (bbm::dot(v, m) > 0);
       
        // Quick exit
        if(bbm::none(mask)) return 0;

        // Lookup precomputed value (rely on clamping of the interpolate method if outside range)
        return bbm::get<"value">(precomputed::holzschuchpacanowski::G1.interpolate<Value>(p, spherical::tanTheta(v)*beta));
      }

      /******************************************************************/
      /*! \brief EPD normalization
       ******************************************************************/
      inline Value compute_normalization(void) const
      {
        Value result(0);

        // Quick bailout
        Mask mask = p > Constants::Epsilon();
        if(bbm::none(mask)) return result;

        // compute on the fly
        if constexpr (NORMALIZATION == epd_normalization::Compute) result = bbm::select(mask, p * Constants::InvPi() * bbm::rcp( bbm::tgamma(bbm::rcp(p)) ), 0.0);

        // use precomputed
        else result = bbm::get<"value">(precomputed::holzschuchpacanowski::distributionNormalization.interpolate<Value>(p));

        // include roughness
        result /= beta*beta;

        // Done.
        return result;
      }
      
      ///////////////////////////////
      //! @{ \name Class Attributes
      ///////////////////////////////
      bsdf_parameter<Value, bsdf_attr::SpecularParameter, 0.003, 0.5, 0.0> beta;  // roughness
      bsdf_parameter<Value, bsdf_attr::SpecularParameter, 0.2, 5.0, 0.0> p;       // kurtosis

      BBM_ATTRIBUTES(beta, p);
      //! @}

      //! \brief Default constructor
      BBM_DEFAULT_CONSTRUCTOR(epd) {}
    };

    BBM_CHECK_CONCEPT(concepts::ndf, epd<config>);
    
  } // end ndf namespace
} // end bbm namespace

#endif /* _BBM_NDF_EPD_H_ */
