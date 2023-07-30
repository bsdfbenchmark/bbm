#ifndef _BBM_COSINEWEIGHTEDLOG_H_
#define _BBM_COSINEWEIGHTEDLOG_H_

#include "core/spherical.h"
#include "bbm/sampledlossfunction.h"
#include "linearizer/spherical_linearizer.h"

/************************************************************************/
/*! \file cosine_weighted_log.h

  \brief The Low et al.'s cosine weighted log BRDF fitting metric from "BRDF
  models for accurate and efficient rendering of glossy surfaces" [Low 2012]:
  https://doi.org/10.1145/2077341.2077350

*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Low et al.'s cosine weighted log error

    Low et al. "BRDF models for accurate and efficient rendering of glossy
    surfaces": https://doi.org/10.1145/2077341.2077350

    A cosine weighted log error on (in,out) BSDF sample. Each sample is
    weighted by sin(theta_in)

    
    Implements: concepts::samplelossfunction
  **********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    struct lowLog_error
  {
    BBM_IMPORT_CONFIG( CONF );

    inline Value operator()(const Vec3d& in, const Vec3d& /*out*/, const Spectrum& value, const Spectrum& reference) const
    {
      auto cosTheta = bbm::max( spherical::cosTheta(in), 0 );
      auto v = bbm::log(1 + value*cosTheta);
      auto r = bbm::log(1 + reference*cosTheta);
      return bbm::hsum( bbm::pow(v-r, 2.0) ) * spherical::sinTheta(in);
    }
  };

  BBM_CHECK_CONCEPT(concepts::samplelossfunction, lowLog_error<config>);

  /**********************************************************************/
  /*! \brief lowLog loss function

    \tparam BSDF = bsdf to optimize
    \tparam REFERENCE = bsdf to match
    \tparam COMPONENT = which bsdf component to optimize
    \tparam UNIT = unit over which to optimize

    lowLog is a sampled loss function that computes the loss over two BSDFs
    using the lowLog error and a spherical sampling of in and out directions
    (spherical_linearizer). Note: *phi_out* is *not* sampled.

    Implements: concepts::sampledlossfunction
  ***********************************************************************/
  template<typename BSDF, typename REFERENCE, bsdf_flag COMPONENT=bsdf_flag::All, unit_t UNIT=unit_t::Radiance>
    requires concepts::matching_config<BSDF, REFERENCE> &&
             concepts::bsdfmodel<BSDF> &&
             concepts::bsdfmodel<REFERENCE>
    struct lowLog : public sampledlossfunction<BSDF, REFERENCE, lowLog_error<get_config<BSDF>>, spherical_linearizer<get_config<BSDF>>, COMPONENT, UNIT>
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
    inline lowLog(const BSDF& bsdf, const REFERENCE& reference,
                  const vec2d<Size_t>& samplesIn,
                  const Size_t& samplesOut,
                  const Vec2d& startIn=0, const Vec2d& endIn=Constants::Hemisphere(),
                  const Scalar& startOut=0, const Scalar& endOut=Constants::Pi(0.5))
      : sampledlossfunction<BSDF, REFERENCE, lowLog_error<Config>, spherical_linearizer<Config>, COMPONENT, UNIT>(bsdf, reference, lowLog_error<Config>(), spherical_linearizer<Config>(samplesIn, vec2d<Size_t>(1, samplesOut), startIn, endIn, startOut, Vec2d(Constants::Pi(2.0), endOut))) {}
  };

  
  /**********************************************************************/
  /*! \brief Bieron and Peers cosine weighted log error

    Bieron and Peers "An Adaptive BRDF Fitting Metric": https://doi.org/10.1111/cgf.14054
    
    A cosine weighted log error on an (in,out) BSDF sample. Each sample is
    weighted by sin(theta_in) * sin(theta_out) * cos(theta_out)

    Implements: concepts::samplelossfunction
  ***********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    struct bieronLog_error
  {
    BBM_IMPORT_CONFIG( CONF );
    
    inline Value operator()(const Vec3d& in, const Vec3d& out, const Spectrum& value, const Spectrum& reference) const
    {
      auto cosTheta = bbm::max( spherical::cosTheta(in), 0 );
      auto v = bbm::log(1 + value*cosTheta);
      auto r = bbm::log(1 + reference*cosTheta);
      return bbm::hsum( bbm::pow(v-r, 2.0) ) * bbm::max( spherical::cosTheta(out), 0 ) * spherical::sinTheta(in) * spherical::sinTheta(out);
    }
  };

  BBM_CHECK_CONCEPT(concepts::samplelossfunction, bieronLog_error<config>);

  /*********************************************************************/
  /*! \brief bieronLog loss function

    \tparam BSDF = bsdf to optimize
    \tparam REFERENCE = bsdf to match
    \tparam COMPONENT = which bsdf component to optimize
    \tparam UNIT = unit over which to optimize

    bieronLog is a sampled loss function that computes the loss over two BSDFs
    using the bieronLog_error and a spherical sampling of in and out directions
    (spherical_linearizer).

    Implements: concepts::sampledlossfunction
   *********************************************************************/
  template<typename BSDF, typename REFERENCE, bsdf_flag COMPONENT=bsdf_flag::All, unit_t UNIT=unit_t::Radiance>
    requires concepts::matching_config<BSDF, REFERENCE> &&
             concepts::bsdfmodel<BSDF> &&
             concepts::bsdfmodel<REFERENCE>
    struct bieronLog : public sampledlossfunction<BSDF, REFERENCE, bieronLog_error<get_config<BSDF>>, spherical_linearizer<get_config<BSDF>>, COMPONENT, UNIT>
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
    inline bieronLog(const BSDF& bsdf, const REFERENCE& reference,
                     const vec2d<Size_t>& samplesIn,
                     const vec2d<Size_t>& samplesOut,
                     const Vec2d& startIn=0, const Vec2d& endIn=Constants::Hemisphere(),
                     const Vec2d& startOut=0, const Vec2d& endOut=Constants::Hemisphere())
      : sampledlossfunction<BSDF, REFERENCE, bieronLog_error<Config>, spherical_linearizer<Config>, COMPONENT, UNIT>(bsdf, reference, bieronLog_error<Config>(), spherical_linearizer<Config>(samplesIn, samplesOut, startIn, endIn, startOut, endOut)) {}
  };


  /*********************************************************************/
  /*! \brief Cosine weighted log error weighted by sin theta of in and out.

    A cosine weighted log error for an (in,out) BSDF sample. Each sample is
    weighted by sin(theta_in) * sin(theta_out).  While this error is the
    re-interpretation of Low et al.'s log error with 'standard' sin theta
    in.out weighting of the error.

    Implements: concepts::samplelossfunction
  ***********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    struct standardLog_error
  {
    BBM_IMPORT_CONFIG( CONF );
    
    inline Value operator()(const Vec3d& in, const Vec3d& out, const Spectrum& value, const Spectrum& reference) const
    {
      auto cosTheta = bbm::max( spherical::cosTheta(in), 0 );
      auto v = bbm::log(1 + value*cosTheta);
      auto r = bbm::log(1 + reference*cosTheta);
      return bbm::hsum( bbm::pow(v-r, 2.0) ) * spherical::sinTheta(in) * spherical::sinTheta(out);
    }
  };

  BBM_CHECK_CONCEPT(concepts::samplelossfunction, standardLog_error<config>);
  
  /**********************************************************************/
  /*! \brief standardLog loss function

    \tparam BSDF = bsdf to optimize
    \tparam REFERENCE = bsdf to match
    \tparam COMPONENT = which bsdf component to optimize
    \tparam UNIT = unit over which to optimize

    standardLog is a sampled loss function that computes the loss over two BSDFs
    using the standardLog_error and a spherical sampling of in and out directions
    (spherical_linearizer).

    Implements: concepts::sampledlossfunction
  **********************************************************************/
  template<typename BSDF, typename REFERENCE, bsdf_flag COMPONENT=bsdf_flag::All, unit_t UNIT=unit_t::Radiance>
    requires concepts::matching_config<BSDF, REFERENCE> &&
             concepts::bsdfmodel<BSDF> &&
             concepts::bsdfmodel<REFERENCE>
    struct standardLog : public sampledlossfunction<BSDF, REFERENCE, standardLog_error<get_config<BSDF>>, spherical_linearizer<get_config<BSDF>>, COMPONENT, UNIT>
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
    inline standardLog(const BSDF& bsdf, const REFERENCE& reference,
                       const vec2d<Size_t>& samplesIn,
                       const vec2d<Size_t>& samplesOut,
                       const Vec2d& startIn=0, const Vec2d& endIn=Constants::Hemisphere(),
                       const Vec2d& startOut=0, const Vec2d& endOut=Constants::Hemisphere())
      : sampledlossfunction<BSDF, REFERENCE, standardLog_error<Config>, spherical_linearizer<Config>, COMPONENT, UNIT>(bsdf, reference, standardLog_error<Config>(), spherical_linearizer<Config>(samplesIn, samplesOut, startIn, endIn, startOut, endOut)) {}
  };
  
} // end bbm namespace

#endif /* _BBM_COSINEWEIGHTEDLOG_H_ */
