#ifndef _BBM_MATH_BACKBONE_CONCEPT_H_
#define _BBM_MATH_BACKBONE_CONCEPT_H_

#include <concepts>

/************************************************************************/
/*! \file math.h

  \brief Extended math functionality

*************************************************************************/

namespace bbm {
  namespace concepts {
    namespace backbone {

      /******************************************************************/
      /*! \brief Concept to check if a type has all bbm math functions

        + fmod(a, b): modulo
        + lerp(a, b, t): linear interpolation
        + exp(a): \f$ e^a \f$
        + log(a): natural logarithm
        + pow(a, b): \f$ a^b \f$
        + sqrt(a): \f$ \sqrt{a} \f$
        + cos(a), sin(a), tan(a): trig functions
        + acos(a), asin(a), atan(a): inverse trig functions
        + atan2(a, b) = atan(a/b)
        + cossin(a) : vec2d(cos(a), sin(a))
        + cosh(a), sinh(a), tanh(a); hyperbolic functions
        + acosh(a), asinh(a), atanh(a); inversehyperbolic functions
        + abs(a): absolute value
        + copysign(a, b) : copy the sign of b and the magnitude of a
        + sign(a) = copysign(1, a)
        + max(a,b), min(a,b)
        + ceil(a), floor(a)
        + round(a)
        + clamp(a, l, u): min(max(a, l), u)
        + safe_sqrt(a) : \f$ \sqrt{ max(a,0) } \f$
        + safe_acos(a) : asin( clamp(a, -1, +1) )
        + safe_asin(a) : acos( clamp(a, -1, +1) )
        + eq, neq: equals and not equals; yields mask_t
        + isnan, isinfinite, isinf; yields mask_t
       ******************************************************************/
      template<typename T>
        concept has_math_functions = requires(T a)
      {
        requires concepts::valid_trait<replace_scalar_t, T, T>;
                
        // General
        { bbm::fmod(a, a) } -> std::convertible_to<T>;
        { bbm::lerp(a, a, a) } -> std::convertible_to<T>;
        
        // Exponentionals
        { bbm::exp(a) } -> std::convertible_to<T>;
        { bbm::log(a) } -> std::convertible_to<T>;
        { bbm::pow(a, a) } -> std::convertible_to<T>;
        { bbm::sqrt(a) } -> std::convertible_to<T>;

        // Trig
        { bbm::cossin(a) } -> std::convertible_to<vec2d<T>>;
        { bbm::cos(a) } -> std::convertible_to<T>;
        { bbm::acos(a) } -> std::convertible_to<T>;
        { bbm::sin(a) } -> std::convertible_to<T>;
        { bbm::asin(a) } -> std::convertible_to<T>;
        { bbm::tan(a) } -> std::convertible_to<T>;
        { bbm::atan(a) } -> std::convertible_to<T>;
        //{ bbm::atan2(a, a) } -> std::convertible_to<T>;

        // Hyperbolic
        { bbm::cosh(a) } -> std::convertible_to<T>;
        { bbm::acosh(a) } -> std::convertible_to<T>;
        { bbm::sinh(a) } -> std::convertible_to<T>;
        { bbm::asinh(a) } -> std::convertible_to<T>;
        { bbm::tanh(a) } -> std::convertible_to<T>;
        { bbm::atanh(a) } -> std::convertible_to<T>;

        // Sign
        { bbm::abs(a) } -> std::convertible_to<T>;
        { bbm::copysign(a, a) } -> std::convertible_to<T>;
        { bbm::sign(a) } -> std::convertible_to<T>;

        // Rounding & Limiting
        { bbm::max(a,a) } -> std::convertible_to<T>;
        { bbm::min(a,a) } -> std::convertible_to<T>;
        { bbm::ceil(a) } -> std::convertible_to<T>;
        { bbm::floor(a) } -> std::convertible_to<T>;
        { bbm::round(a) } -> std::convertible_to<T>;
        { bbm::clamp(a,a,a) } ->std::convertible_to<T>;
        
        // Safe versions
        { bbm::safe_sqrt(a) } -> std::convertible_to<T>;
        { bbm::safe_asin(a) } -> std::convertible_to<T>;
        { bbm::safe_acos(a) } -> std::convertible_to<T>;

        // Comparisons
        { bbm::eq(a,a) } -> std::convertible_to<mask_t<T>>;
        { bbm::neq(a,a) } -> std::convertible_to<mask_t<T>>;

        { bbm::isnan(a) } -> std::convertible_to<mask_t<T>>;
        { bbm::isinf(a) } -> std::convertible_to<mask_t<T>>;
        { bbm::isfinite(a) } -> std::convertible_to<mask_t<T>>;
      };    
      
    } // end backbone namespace
  } // end concepts namespace
} // end bbm namespace


#endif /* _BBM_MATH_BACKBONE_CONCEPT_H_ */
