#ifndef _BBM_COSINEWEIGHTEDL2_H_
#define _BBM_COSINEWEIGHTEDL2_H_

#include "core/spherical.h"
#include "bbm/sampledlossfunction.h"
#include "linearizer/spherical_linearizer.h"

/************************************************************************/
/*! \file cosine_weighted_l2.h

  A cosine weighted L2 loss functions
*************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief Standard cosine weighted l2 error

    A cosine weighted l2 error for an (in,out) BSDF sample. Each sample is
    weighted by sin(theta_in) * sin(theta_out)

    Implements: concepts::samplelossfunction
  ***********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    struct nganL2_error
  {
    BBM_IMPORT_CONFIG( CONF );
    
    inline Value operator()(const Vec3d& in, const Vec3d& out, const Spectrum& value, const Spectrum& reference) const
    {
      return bbm::hsum(bbm::pow( (value - reference) * bbm::max( spherical::cosTheta(in), 0 ), 2.0))
        * spherical::sinTheta(in) * spherical::sinTheta(out);
    }
  };

  BBM_CHECK_CONCEPT(concepts::samplelossfunction, nganL2_error<config>);
  
  /**********************************************************************/
  /*! \brief nganL2 loss function

    \tparam BSDF = bsdf to optimize
    \tparam REFERENCE = bsdf to match
    \tparam COMPONENT = which bsdf component to optimize
    \tparam UNIT = unit over which to optimize

    nganL2 is a sampled loss function that computes the loss over two BSDFs
    using the nganL2_error and a spherical sampling of in and out directions
    (spherical_linearizer).

    Implements: concepts::sampledlossfunction
  **********************************************************************/
  template<typename BSDF, typename REFERENCE, bsdf_flag COMPONENT=bsdf_flag::All, unit_t UNIT=unit_t::Radiance>
    requires concepts::matching_config<BSDF, REFERENCE> &&
             concepts::bsdfmodel<BSDF> &&
             concepts::bsdfmodel<REFERENCE>
    struct nganL2 : public sampledlossfunction<BSDF, REFERENCE, nganL2_error<get_config<BSDF>>, spherical_linearizer<get_config<BSDF>>, COMPONENT, UNIT>
  {
    BBM_IMPORT_CONFIG( get_config<BSDF> );

    /********************************************************************/
    /*! \brief Constructor

      \param bsdf = bsdf to optimize
      \param reference  = bsdf to match
      \param samplesIn = number of (phi, theta) samples for the 'in' direction
      \param samplesOut = number of (phi, theta) samples for the 'out' direction
      \param startIn = (phi,theta) angles to start sampling the 'in' direction (default: (0,0))
      \param endIn = (phi,theta) angles to end sampling the 'in' direction (defailt: Hemisphere)
      \param startOut = (phi,theta) angles to start sampling the 'out' direction (default: (0,0))
      \param endOut = (phi,theta) angles to end sampling the 'out' direction (default: Hemisphere)
    *********************************************************************/
    inline nganL2(const BSDF& bsdf, const REFERENCE& reference,
                  const vec2d<Size_t>& samplesIn,
                  const vec2d<Size_t>& samplesOut,
                  const Vec2d& startIn=0, const Vec2d& endIn=Constants::Hemisphere(),
                  const Vec2d& startOut=0, const Vec2d& endOut=Constants::Hemisphere())
      : sampledlossfunction<BSDF, REFERENCE, nganL2_error<Config>, spherical_linearizer<Config>, COMPONENT, UNIT>(bsdf, reference, nganL2_error<Config>(), spherical_linearizer<Config>(samplesIn, samplesOut, startIn, endIn, startOut, endOut)) {}
  };

  
  /**********************************************************************/
  
  
  /**********************************************************************/
  /*! \brief Low et al. cosine weighted l2 error

    Low et al. "BRDF models for accurate and efficient rendering of glossy
    surfaces": https://doi.org/10.1145/2077341.2077350

    A cosine weighted l2 error on (in,out) BSDF sample. Each sample is
    weighted by sin(theta_in)

    Implements: concepts::samplelossfunction
  ***********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    struct lowL2_error
  {
    BBM_IMPORT_CONFIG( CONF );
    
    inline Value operator()(const Vec3d& in, const Vec3d& /*out*/, const Spectrum& value, const Spectrum& reference) const
    {
      return bbm::hsum(bbm::pow( (value - reference) * bbm::max( spherical::cosTheta(in), 0 ), 2.0 ))
        * spherical::sinTheta(in);
    }
  };

  BBM_CHECK_CONCEPT(concepts::samplelossfunction, lowL2_error<config>);
  
  /**********************************************************************/
  /*! \brief lowL2 loss function

    \tparam BSDF = bsdf to optimize
    \tparam REFERENCE = bsdf to match
    \tparam COMPONENT = which bsdf component to optimize
    \tparam UNIT = unit over which to optimize

    lowL2 is a sampled loss function that computes the loss over two BSDFs
    using the lowL2_error and a spherical sampling of in and out directions
    (spherical_linearizer). Note: *phi_out* is *not* sampled.

    Implements: concepts::sampledlossfunction
  **********************************************************************/
  template<typename BSDF, typename REFERENCE, bsdf_flag COMPONENT=bsdf_flag::All, unit_t UNIT=unit_t::Radiance>
    requires concepts::matching_config<BSDF, REFERENCE> &&
             concepts::bsdfmodel<BSDF> &&
             concepts::bsdfmodel<REFERENCE>
    struct lowL2 : public sampledlossfunction<BSDF, REFERENCE, lowL2_error<get_config<BSDF>>, spherical_linearizer<get_config<BSDF>>, COMPONENT, UNIT>
  {
    BBM_IMPORT_CONFIG( get_config<BSDF> );

    /********************************************************************/
    /*! \brief Constructor

      \param bsdf = bsdf to optimize
      \param reference  = bsdf to match
      \param samplesIn = number of (phi, theta) samples for the 'in' direction
      \param samplesOut = number of *theta* samples for the 'out' direction
      \param startIn = (phi,theta) angles to start sampling the 'in' direction (default: (0,0))
      \param endIn = (phi,theta) angles to end sampling the 'in' direction (defailt: Hemisphere)
      \param startOut = *theta* angle to start sampling the 'out' direction (default: 0)
      \param endOut = *theta* angle to end sampling the 'out' direction (default: Pi(0.5))
    *********************************************************************/
    inline lowL2(const BSDF& bsdf, const REFERENCE& reference,
                 const vec2d<Size_t>& samplesIn,
                 const Size_t& samplesOut,
                 const Vec2d& startIn=0, const Vec2d& endIn=Constants::Hemisphere(),
                 const Scalar& startOut=0, const Scalar& endOut=Constants::Pi(0.5))
      : sampledlossfunction<BSDF, REFERENCE, lowL2_error<Config>, spherical_linearizer<Config>, COMPONENT, UNIT>(bsdf, reference, lowL2_error<Config>(), spherical_linearizer<Config>(samplesIn, vec2d<Size_t>(1, samplesOut), startIn, endIn, startOut, Vec2d(Constants::Pi(2.0), endOut))) {}
  };

    
  /**********************************************************************/
  
  
  /**********************************************************************/
  /*! \brief Bieron and Peers cosine weighted l2 error

    Bieron and Peers "An Adaptive BRDF Fitting Metric": https://doi.org/10.1111/cgf.14054
    
    A cosine weighted l2 error on an (in,out) BSDF sample. Each sample is
    weighted by sin(theta_in) * sin(theta_out) * cos(theta_out)

    Implements: concepts::samplelossfunction
  ***********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    struct bieronL2_error
  {
    BBM_IMPORT_CONFIG( CONF );
    
    inline Value operator()(const Vec3d& in, const Vec3d& out, const Spectrum& value, const Spectrum& reference) const
    {
      return bbm::hsum(bbm::pow( (value - reference) * bbm::max( spherical::cosTheta(in), 0 ), 2.0))
        * bbm::max( spherical::cosTheta(out), 0 )
        * spherical::sinTheta(in) * spherical::sinTheta(out);
    }
  };

  BBM_CHECK_CONCEPT(concepts::samplelossfunction, bieronL2_error<config>);
  
  /*********************************************************************/
  /*! \brief bieronL2 loss function

    \tparam BSDF = bsdf to optimize
    \tparam REFERENCE = bsdf to match
    \tparam COMPONENT = which bsdf component to optimize
    \tparam UNIT = unit over which to optimize

    bieronL2 is a sampled loss function that computes the loss over two BSDFs
    using the bieronL2_error and a spherical sampling of in and out directions
    (spherical_linearizer).

    Implements: concepts::sampledlossfunction
   *********************************************************************/
  template<typename BSDF, typename REFERENCE, bsdf_flag COMPONENT=bsdf_flag::All, unit_t UNIT=unit_t::Radiance>
    requires concepts::matching_config<BSDF, REFERENCE> &&
             concepts::bsdfmodel<BSDF> &&
             concepts::bsdfmodel<REFERENCE>
    struct bieronL2 : public sampledlossfunction<BSDF, REFERENCE, bieronL2_error<get_config<BSDF>>, spherical_linearizer<get_config<BSDF>>, COMPONENT, UNIT>
  {
    BBM_IMPORT_CONFIG( get_config<BSDF> );

    /********************************************************************/
    /*! \brief Constructor

      \param bsdf = bsdf to optimize
      \param reference  = bsdf to match
      \param samplesIn = number of (phi, theta) samples for the 'in' direction
      \param samplesOut = number of (phi, theta) samples for the 'out' direction
      \param startIn = (phi,theta) angles to start sampling the 'in' direction (default: (0,0))
      \param endIn = (phi,theta) angles to end sampling the 'in' direction (defailt: Hemisphere)
      \param startOut = (phi,theta) angles to start sampling the 'out' direction (default: (0,0))
      \param endOut = (phi,theta) angles to end sampling the 'out' direction (default: Hemisphere)
    *********************************************************************/
    inline bieronL2(const BSDF& bsdf, const REFERENCE& reference,
                    const vec2d<Size_t>& samplesIn,
                    const vec2d<Size_t>& samplesOut,
                    const Vec2d& startIn=0, const Vec2d& endIn=Constants::Hemisphere(),
                    const Vec2d& startOut=0, const Vec2d& endOut=Constants::Hemisphere())
      : sampledlossfunction<BSDF, REFERENCE, bieronL2_error<Config>, spherical_linearizer<Config>, COMPONENT, UNIT>(bsdf, reference, bieronL2_error<Config>(), spherical_linearizer<Config>(samplesIn, samplesOut, startIn, endIn, startOut, endOut)) {}
  };
    
  
} // end bbm namespace

#endif /* _BBM_COSINEWEIGHTEDL2_H_ */
