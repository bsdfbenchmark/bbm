#ifndef _BBM_SPHERICAL_LINEARIZER_H_
#define _BBM_SPHERICAL_LINEARIZER_H_

#include "concepts/macro.h"
#include "concepts/inout_linearizer.h"

#include "core/spherical.h"

#include "bbm/config.h"
#include "bbm/vec3dpair.h"


/***********************************************************************/
/* \file spherical_linearizer.h

   \brief Linearize the spherical coordinates by linearizing a regular
   sampling of (theta,phi) coordinates for both in and out.

   Follows concepts/inout_linearizer.h
************************************************************************/

namespace bbm {

  template<typename CONF> requires concepts::config<CONF>
    struct spherical_linearizer
  {
    BBM_IMPORT_CONFIG( CONF );
    
    /******************************************************************/
    /*! \brief Constructor

      \param samplesIn = number vec2d<size_t>(phi, theta) samples for incident directions
      \param samplesOut = number vec2d<size_t>(phi, theta) samples for outgoing directions
      \param startIn = start vec2d<Scalar>(phi,theta) for incident directions (default 0)
      \param endIn = end vec2d<Scalar(phi,theta) for incident directions (default HEMISPHERE)
      \param startOut = start vec2d<Scalar>(phi,theta) for outgoing directions (default 0)
      \param endOut = end vec2d<Sccalar>(phi,theta) for outgoing directions (default HEMISPHERE)
    ********************************************************************/
    inline spherical_linearizer(const vec2d<Size_t>& samplesIn,
                                const vec2d<Size_t>& samplesOut,
                                const Vec2d& startIn = 0,
                                const Vec2d& endIn = Constants::Hemisphere(),
                                const Vec2d& startOut = 0,
                                const Vec2d& endOut = Constants::Hemisphere())
      : _samplesIn(samplesIn), _samplesOut(samplesOut), _startIn(startIn), _endIn(endIn), _startOut(startOut), _endOut(endOut), _sizeIn(endIn - startIn), _sizeOut(endOut - startOut) { }

    /********************************************************************/
    /*! \brief Copy Constructor
     ********************************************************************/
    spherical_linearizer(const spherical_linearizer& src) : _samplesIn(src._samplesIn), _samplesOut(src._samplesOut), _startIn(src._startIn), _endIn(src._endIn), _startOut(src._startOut), _endOut(src._endOut), _sizeIn(src._sizeIn), _sizeOut(src._sizeOut) {}

    /*******************************************************************/
    /*! \brief Size of the linearized 'array' of direction pairs
    ********************************************************************/
    Size_t size(void) const
    {
      return _samplesIn[0] * _samplesIn[1] * _samplesOut[0] * _samplesOut[1];
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
    Vec3dPair operator()(Size_t idx, Mask mask=true) const
    {
      Vec3dPair result = {0,0};

      // check bounds
      mask &= (idx < size());
      if(bbm::none(mask)) return result;
            
      // compute spherical indices
      vec2d<Size_t> in_coord, out_coord;
      Size_t temp_idx = idx;

      out_coord[1] = temp_idx % _samplesOut[1];
      temp_idx /= _samplesOut[1];
            
      out_coord[0] = temp_idx % _samplesOut[0];
      temp_idx /= _samplesOut[0];
      
      in_coord[1] = temp_idx % _samplesIn[1];
      temp_idx /= _samplesIn[1];

      in_coord[0] = temp_idx;
      
      // compute spherical angles (include equator/end samples)
      vec2d<Size_t> sIn( _samplesIn[0], bbm::max( _samplesIn[1] - 1, Size_t(1)) );
      vec2d<Size_t> sOut( _samplesOut[0], bbm::max( _samplesOut[1] - 1, Size_t(1)) );

      Vec2d sph_in = bbm::cast<Vec2d>(in_coord * _sizeIn / sIn) + _startIn;
      Vec2d sph_out = bbm::cast<Vec2d>(out_coord * _sizeOut / sOut) + _startOut;

      // Create Vec3dPair
      result.in = spherical::convert(sph_in);
      result.out = spherical::convert(sph_out);

      // handle round-off errors close to zero
      result.in  = bbm::select(bbm::abs(result.in) < Constants::Epsilon(), 0, result.in);
      result.out  = bbm::select(bbm::abs(result.out) < Constants::Epsilon(), 0, result.out);
      
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
      // get normalized spherical coordinates
      Vec2d sph_in = spherical::convert(in);
      Vec2d sph_out = spherical::convert(out);

      // check bounds
      mask &= bbm::all((sph_in > _startIn-Constants::Epsilon()) && (sph_in < _endIn+Constants::Epsilon()) && (sph_out > _startOut-Constants::Epsilon()) && (sph_out < _endOut - Constants::Epsilon()));

      // quick bail out
      if(bbm::none(mask)) return size();
      
      // convert to local index and clamp
      vec2d<Size_t> in_coord = bbm::cast<vec2d<Size_t>>(bbm::clamp((sph_in - _startIn) * _samplesIn / _sizeIn, 0, _samplesIn-1));
      vec2d<Size_t> out_coord = bbm::cast<vec2d<Size_t>>(bbm::clamp((sph_out - _startOut) * _samplesOut / _sizeOut, 0, _samplesOut-1));

      // Done.
      return bbm::select(mask, ((in_coord[0] * _samplesIn[1] + in_coord[1]) * _samplesOut[0] + out_coord[0]) * _samplesOut[1] + out_coord[1], size());
    }
      
  private:
    ///////////////////////
    // Class Attributes
    ///////////////////////
    vec2d<Size_t> _samplesIn, _samplesOut;
    Vec2d _startIn, _endIn;
    Vec2d _startOut, _endOut;
    Vec2d _sizeIn, _sizeOut;
  };

  BBM_CHECK_CONCEPT(concepts::inout_linearizer, spherical_linearizer<config>)
  
} // end bbm namespace

#endif /* _BBM_SPHERICAL_LINEARIZER_H_ */
