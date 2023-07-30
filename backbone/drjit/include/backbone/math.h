#ifndef _BBM_DRJIT_MATH_H_
#define _BBM_DRJIT_MATH_H_

#include <cmath>

#include "drjit/math.h"

#include "util/poly.h"
#include "backbone/vec.h"

/************************************************************************/
/*! \file math.h

  \brief Connect bbm math functions to the corresponding DrJIT methods.

  Satisfies: concepts::backbone::has_math_functions

*************************************************************************/

namespace backbone {

  /**********************************************************************/
  /*! @{ \name Map math operations to DrJIT functions
   **********************************************************************/
  using drjit::exp;
  using drjit::log;
  using drjit::sin;
  using drjit::asin;
  using drjit::cos;
  using drjit::acos;
  using drjit::tan;
  using drjit::atan;
  using drjit::atan2;
  using drjit::sinh;
  using drjit::asinh;
  using drjit::cosh;
  using drjit::acosh;
  using drjit::tanh;
  using drjit::atanh;
  using drjit::ceil;
  using drjit::floor;
  using drjit::round;
  using drjit::clamp;
  using drjit::fmod;
  using drjit::copysign;
  using drjit::sign;
  using drjit::lerp;
  using drjit::abs;
  using drjit::sqrt;
  using drjit::cbrt;
  using drjit::pow;
  using drjit::safe_sqrt;
  using drjit::safe_asin;
  using drjit::safe_acos;
  using drjit::erf;
  using drjit::erfinv;
  using drjit::tgamma;
  using drjit::lgamma;
  using drjit::eq;
  using drjit::neq;
  using drjit::rcp;
  using drjit::rsqrt;
  using drjit::isnan;
  using drjit::isinf;
  using drjit::isfinite;
  //! @}
  
  // clean up macro
  #undef BBM_DRJIT_MATHOP

  /********************************************************************/
  /*! \brief max between two variables
   ********************************************************************/
  template<typename T, typename U>
    inline auto max(T&& t, U&& u)
  {
    return drjit::select(t > u, std::forward<T>(t), std::forward<U>(u));
  }

  /********************************************************************/
  /*! \brief min between two variables
   ********************************************************************/
  template<typename T, typename U>
    inline auto min(T&& t, U&& u)
  {
    return drjit::select(t < u, std::forward<T>(t), std::forward<U>(u));
  }
  
  /********************************************************************/
  /*! \brief erfc

    TODO: make more robust
   ********************************************************************/
  template<typename T>
    inline T erfc(const T& t)
  {
    return drjit::scalar_t<T>(1) - drjit::erf(t);
  }
  
  /********************************************************************/
  /*! \brief cossin

    \param a = angle
    \returns the cos and sin in a vec2d
   ********************************************************************/
  template<typename T>
    vec2d<T> cossin(const T& a)
  {
    auto tmp = drjit::sincos(a);
    return {tmp.second, tmp.first};
  }

} // end backbone namespace

#endif /* _BBM_DRJIT_MATH_H_ */
