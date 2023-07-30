#ifndef _BBM_MERL_LINEARIZER_H_
#define _BBM_MERL_LINEARIZER_H_

#include "concepts/macro.h"
#include "concepts/inout_linearizer.h"

#include "bbm/config.h"
#include "bbm/vec3dpair.h"

/************************************************************************/
/*! \file merl_linearizer.h

  \brief Linearize the sphere of in-out directions according to the
  encoding used in the MERL BRDF database.

  Follows concepts/inout_linearizer.h"
*************************************************************************/

namespace bbm {

  template<typename CONF> requires concepts::config<CONF>
    struct merl_linearizer
  {
    BBM_IMPORT_CONFIG( CONF );

    /********************************************************************/
    /*! \brief Constructor
     ********************************************************************/
    merl_linearizer(const vec2d<Size_t>& h = vec2d<Size_t>(1,90), const vec2d<Size_t>& d=vec2d<Size_t>(180,90)) : _samplesH(h), _samplesD(d) {}

    /*******************************************************************/
    /*! \brief Size of the linearized 'array' of direction pairs
    ********************************************************************/
    Size_t size(void) const
    {
      return _samplesD[0] * _samplesD[1] * _samplesH[1];
    }

    /********************************************************************/
    /*! \brief The in-out coordinate of the idx-th sample

      \param idx = index of the sample (in [0, samples()-1]
      \returns a Ve3dPair of the in and out direction of the
      sample. Will return {0,0} if the index is not valid.
      
      Note: the sample direction is placed at the center of the index's bin, 
      except for the top and bottom, where it is placed at the center of the
      top and bottom edge respectively.
    *********************************************************************/
    Vec3dPair operator()(const Size_t& idx, Mask mask=true) const
    {
      Vec3dPair result = {0,0};
      
      // check bounds
      mask &= (idx < size());
      if(bbm::none(mask)) return result;

      // compute halfway/difference indices
      vec2d<Size_t> idxH(0);
      vec2d<Size_t> idxD(0);
      Size_t temp_idx = idx;

      spherical::phi(idxD) = temp_idx % spherical::phi(_samplesD);
      temp_idx /= spherical::phi(_samplesD);

      spherical::theta(idxD) = temp_idx % spherical::theta(_samplesD);
      temp_idx /= spherical::theta(_samplesD);

      spherical::theta(idxH) = temp_idx;

      // compute halfway/difference vectors
      Vec2d half_sph = bbm::pow(bbm::cast<Vec2d>(idxH) / _samplesH, 2.0) * (0.5 * Constants::Sphere());
      Vec2d diff_sph = (bbm::cast<Vec2d>(idxD) / _samplesD) * (0.5 * Constants::Sphere());

      std::tie(result.in, result.out) = convertFromHalfwayDifference( spherical::convert(half_sph), spherical::convert(diff_sph) );

      // make sure round off errors don't push us below the horizon
      vec::z(result.in) = bbm::max(vec::z(result.in), 0);
      vec::z(result.out) = bbm::max(vec::z(result.out), 0);

      // Done.
      return bbm::select(mask, result, Vec3dPair{0,0});
    }

    /********************************************************************/
    /*! \brief Map a coordinate to an linear index

      \param in = incident direction
      \param out = outgoing direction
      \returns linearized index

      Invalid querries are set to 'size()'.
    *********************************************************************/
    Size_t operator()(const Vec3d& in, const Vec3d& out, Mask mask=true) const
    {
      // above surface?
      mask &= ((vec::z(in) >= 0) && (vec::z(out) >= 0));

      // quick bail out (all samples are invalid)
      if(bbm::none(mask)) return size();

      // compute index in (thetaH, thetaD, phiD)
      auto HalfDiff = convertToHalfwayDifference(in, out);
      auto halfway = spherical::convert(HalfDiff.first);
      auto difference = spherical::convert(HalfDiff.second);
      
      // when in ~= out, phi(difference) is unstable
      spherical::phi(difference) = bbm::select( bbm::dot(in,out) > 1-Constants::Epsilon(), 0, spherical::phi(difference) );

      // reciprocity for phi(difference)
      spherical::phi(difference) = bbm::select( (spherical::phi(difference) >= Constants::Pi()), spherical::phi(difference) - Constants::Pi(), spherical::phi(difference) );

      // add epsilon to counter round-off errors
      vec2d<Size_t> idxD = bbm::cast<vec2d<Size_t>>((difference/Constants::Sphere(0.5) + Constants::Epsilon()) * _samplesD);
      vec2d<Size_t> idxH = bbm::cast<vec2d<Size_t>>(bbm::safe_sqrt(halfway/Constants::Sphere(0.5) + Constants::Epsilon()) * _samplesH);
      
      // clamp to ensure it remain inside range
      idxD = bbm::clamp(idxD, 0, _samplesD - 1);
      idxH = bbm::clamp(idxH, 0, _samplesH - 1); 
      
      // Merge indices
      return bbm::select(mask, (spherical::theta(idxH) * spherical::theta(_samplesD) + spherical::theta(idxD)) * spherical::phi(_samplesD) + spherical::phi(idxD), size());
    }

  private:
    /////////////////////
    // Class Attributes
    /////////////////////
    vec2d<Size_t> _samplesH;
    vec2d<Size_t> _samplesD;
  };
  
  BBM_CHECK_CONCEPT(concepts::inout_linearizer, merl_linearizer<config>);
  
} // end bbm namespace


#endif /* _BBM_MERL_LINEARIZER_H_ */
