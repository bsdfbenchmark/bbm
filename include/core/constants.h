#ifndef _BBM_CONSTANTS_H_
#define _BBM_CONSTANTS_H_

/***********************************************************************/
/*! \file constant.h

  \brief Definition of convenient constants
***********************************************************************/

#include <limits>
#include <numbers>
#include "core/vec.h"

namespace bbm {

  template<typename T>
  struct constants {
    static constexpr T Epsilon(void) { return std::numeric_limits<T>::epsilon(); }
    static constexpr T Pi(T scale=1) { return scale * std::numbers::pi; }
    static constexpr T InvPi(T scale=1) { return scale * std::numbers::inv_pi; }
    static constexpr T Pi2(T scale=1) { return scale * Pi()*Pi(); }
    static constexpr T InvSqrtPi(T scale=1) { return scale * std::numbers::inv_sqrtpi; }
    static constexpr vec2d<T> Sphere(T scale=1) { return {Pi(2*scale), Pi(scale)}; }
    static constexpr vec2d<T> Hemisphere(T scale=1)  { return {Pi(2*scale), Pi(0.5*scale)}; }
  };
  
} // end bbm namespace

#endif /* _BBM_CONSTANTS_H_ */
