#ifndef _BBM_ENOKI_MATH_H_
#define _BBM_ENOKI_MATH_H_

#include "backbone/vec.h"
#include "backbone/array.h"
#include "enoki/special.h"
#include "core/attribute.h"

/************************************************************************/
/*! \file math.h

  \brief Connect the bbm math functions to the corresponding enoki methods.

  Satisfies concepts::backbone::has_math_functions

*************************************************************************/

namespace backbone {

  /**********************************************************************/
  /*! \brief Helper Macro for mapping math operations to enoki. Takes in
      account to pass the 'value' of variables and to cast scalars to the
      correct type to preserve differentiability.
  ***********************************************************************/
#define BBM_ENOKI_MATHOP(OpName)                                         \
  template<typename... Ts> requires requires(const Ts&... t) {{enoki::OpName(bbm::value(t)...)};} \
    inline constexpr auto OpName(const Ts&... t)                         \
  {                                                                      \
    constexpr bool has_floating_scalar = (( std::is_scalar_v< std::decay_t<decltype(bbm::value(t))> > && std::is_floating_point_v< std::decay_t<decltype(bbm::value(t))> > ) || ...); \
                                                                         \
    if constexpr (has_floating_scalar)                                   \
    {                                                                    \
      using base_type = decltype( (std::declval<std::conditional_t< std::is_scalar_v< std::decay_t<decltype(bbm::value(t))> >, float, decltype(bbm::value(t)) >>() + ...) ); \
      using type = enoki::scalar_t<base_type>;                           \
      return enoki::OpName( std::conditional_t<std::is_scalar_v<std::decay_t<decltype(bbm::value(t))>>, type, decltype(bbm::value(t))>(bbm::value(t))... ); \
    }                                                                    \
    else return enoki::OpName( bbm::value(t)... );                       \
  }                                                                      \
  
    /********************************************************************/
    /*! @{ \name Map math operations to Enoki functions
      *******************************************************************/
    BBM_ENOKI_MATHOP(exp);
    BBM_ENOKI_MATHOP(log);
    BBM_ENOKI_MATHOP(sin);
    BBM_ENOKI_MATHOP(asin);
    BBM_ENOKI_MATHOP(cos);
    BBM_ENOKI_MATHOP(acos);
    BBM_ENOKI_MATHOP(tan);
    BBM_ENOKI_MATHOP(atan);
    BBM_ENOKI_MATHOP(atan2);
    BBM_ENOKI_MATHOP(sinh);
    BBM_ENOKI_MATHOP(asinh);
    BBM_ENOKI_MATHOP(cosh);
    BBM_ENOKI_MATHOP(acosh);
    BBM_ENOKI_MATHOP(tanh);
    BBM_ENOKI_MATHOP(atanh);
    BBM_ENOKI_MATHOP(ceil);
    BBM_ENOKI_MATHOP(floor);
    BBM_ENOKI_MATHOP(round);
    BBM_ENOKI_MATHOP(clamp);
    BBM_ENOKI_MATHOP(fmod);
    BBM_ENOKI_MATHOP(copysign);
    BBM_ENOKI_MATHOP(sign);
    BBM_ENOKI_MATHOP(lerp);
    BBM_ENOKI_MATHOP(abs);
    BBM_ENOKI_MATHOP(sqrt);
    BBM_ENOKI_MATHOP(cbrt);
    BBM_ENOKI_MATHOP(pow);
    BBM_ENOKI_MATHOP(max);
    BBM_ENOKI_MATHOP(min);
    BBM_ENOKI_MATHOP(safe_sqrt);
    BBM_ENOKI_MATHOP(safe_asin);
    BBM_ENOKI_MATHOP(safe_acos);
    BBM_ENOKI_MATHOP(erf);
    BBM_ENOKI_MATHOP(erfc);
    BBM_ENOKI_MATHOP(erfinv);
    BBM_ENOKI_MATHOP(tgamma);
    BBM_ENOKI_MATHOP(lgamma);
    BBM_ENOKI_MATHOP(eq);
    BBM_ENOKI_MATHOP(neq);
    BBM_ENOKI_MATHOP(rcp);
    BBM_ENOKI_MATHOP(rsqrt);
    BBM_ENOKI_MATHOP(isnan);
    BBM_ENOKI_MATHOP(isinf);
    BBM_ENOKI_MATHOP(isfinite);
    //! @}

    // clean up maco
    #undef BBM_ENOKI_MATHOP

    /********************************************************************/
    /*! \brief cossin

      \param a = angle
      \returns the cos and sin in a vec2d
     ********************************************************************/
    template<typename T>
    vec2d<T> cossin(const T& a)
    {
      auto tmp = enoki::sincos(a);
      return {tmp.second, tmp.first};
    }
  
} // end backbone namespace


#endif /* _BBM_ENOKI_MATH_H_ */
