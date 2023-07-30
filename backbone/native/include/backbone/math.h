#ifndef _BBM_NATIVE_MATH_H_
#define _BBM_NATIVE_MATH_H_

#include <cmath>
#include <utility>

#include "concepts/util.h"
#include "util/iterator_util.h"
#include "util/poly.h"
#include "backbone/vec.h"


/************************************************************************/
/*! \file math.h

  \brief Extend basic math function to scalar STL and backbone::array types
  
  
  Satisfies concepts::backbone::has_math_functions

*************************************************************************/

namespace backbone {

  /*** Implementation details for result_t ***/
  namespace detail {
    template<typename T, typename... U>
      struct result_impl
    {
      using type = typename result_impl<T, typename result_impl<U...>::type>::type;
    };
    
    template<typename T, typename U> requires (!requires(T a, U b) { {a+b}; })
      struct result_impl<T, U>
    {
      using type = std::decay_t<T>;
    };


    template<typename T, typename U> requires requires(T a, U b) { {a+b}; }
      struct result_impl<T, U>
    {
      using type = decltype( std::declval<T>() + std::declval<U>() );
    };
  }

  //! \brief determine the mathematical result type; reverts to first argument if '+' is not defined on T...
  template<typename... T>
    using result_t = typename detail::result_impl<T...>::type;
  
  /**********************************************************************/
  /*! @{ \name Basic STL math functions
   **********************************************************************/
  using std::exp;
  using std::log;
  using std::sin;
  using std::asin;
  using std::cos;
  using std::acos;
  using std::tan;
  using std::atan;
  using std::sinh;
  using std::asinh;
  using std::cosh;
  using std::acosh;
  using std::tanh;
  using std::atanh;
  using std::ceil;
  using std::floor;
  using std::round;
  using std::copysign;
  using std::abs;
  using std::sqrt;
  using std::cbrt;
  using std::erf;
  using std::erfc;
  using std::tgamma;
  using std::lgamma;
  using std::isnan;
  using std::isinf;
  using std::isfinite;
  //! @}
  
  /**********************************************************************/
  /*! @{ \name STL math extensions
   **********************************************************************/ 
  template<typename T, typename U> requires (!is_array_v<T>) && requires(T a, U b) { { std::atan2(result_t<T, U>(a), result_t<T,U>(b)) }; }
    inline constexpr auto atan2(T a, U b) { return std::atan2(result_t<T,U>(a), result_t<T,U>(b)); }
  
  template<typename T, typename U> requires (!is_array_v<T>) && requires(T a, U b) { { std::fmod(result_t<T, U>(a), result_t<T,U>(b)) }; }
    inline constexpr auto fmod(T a, U b) { return std::fmod(result_t<T,U>(a), result_t<T,U>(b)); }
  
  template<typename T, typename U, typename V> requires (!is_array_v<T>) && requires(T a, U b, V c) { { std::lerp(result_t<T, U, V>(a), result_t<T,U,V>(b), result_t<T,U,V>(c)) }; }
    inline constexpr auto lerp(T a, U b, V c) { return std::lerp(result_t<T,U,V>(a), result_t<T,U,V>(b), result_t<T,U,V>(c)); }
  
  template<typename T, typename U> requires (!is_array_v<T>) && requires(T a, U b) { { std::pow(result_t<T, U>(a), result_t<T,U>(b)) }; }
    inline constexpr auto pow(T a, U b) { return std::pow(result_t<T,U>(a), result_t<T,U>(b)); }
  
  template<typename T, typename U> requires (!is_array_v<T>) && requires(T a, U b) { { std::fmax(result_t<T,U>(a), result_t<T,U>(b)) };  }
    inline constexpr auto max(T a, U b) { return std::fmax(result_t<T,U>(a),result_t<T,U>(b)); }

  template<typename T, typename U> requires (!is_array_v<T>) && requires(T a, U b) { { std::fmin(result_t<T,U>(a), result_t<T,U>(b)) };  }
    inline constexpr auto min(T a, U b) { return std::fmin(result_t<T,U>(a),result_t<T,U>(b)); }

  template<typename T, typename L, typename U> requires (!is_array_v<T>) && requires(const T& a, const L& l, const U& u) { { std::clamp(a, T(l), T(u)) }; }
    inline constexpr auto clamp(const T& a, const L& l, const U& u) { return std::clamp(a, T(l), T(u)); }

  template<typename T>
    inline constexpr auto rcp(T a) { return 1 / a; }

  template<typename T>  requires requires(const T& a) {{std::sqrt(a)};}
    inline constexpr auto rsqrt(T a) { return rcp(sqrt(a)); }

  template<typename T> requires requires(const T& a) {{std::log(a)};}
    inline constexpr auto erfinv(T a)
  {
    T w = -std::log((1.0-a) * (1.0+a));
    if(w < 5) return bbm::poly(w - 2.5, 1.50140941, 0.246640727, -0.00417768164, -0.00125372503, 0.00021858087, -4.39150654e-06, -3.5233877e-06, 3.43273939e-07, 2.81022636e-08) * a;
    else return bbm::poly(sqrt(w) - 3.0, 2.83297682, 1.00167406, 0.00943887047, -0.0076224613, 0.00573950773, -0.00367342844, 0.00134934322, 0.000100950558, -0.000200214257) * a;
  }
  
  template<typename T> requires requires(T a) { { std::copysign(T(1), a) }; }
    inline constexpr auto sign(T a) { return std::copysign(T(1), a); }

  template<typename T> requires requires(T a) { { std::cos(a) }; { std::sin(a) }; }
    inline constexpr vec2d<T> cossin(T a) { return vec2d<T>(std::cos(a), std::sin(a)); }

  template<typename T> requires requires(T a) { { std::sqrt(std::fmax(a,T(0))) }; }
    inline constexpr auto safe_sqrt(T a) { return std::sqrt( std::max(a, T(0)) ); }

  template<typename T> requires requires(T a) { { std::asin(std::fmin(T(1), std::fmax(T(-1), a))) }; }
    inline constexpr auto safe_asin(T a) { return std::asin(std::fmin(T(1), std::fmax(T(-1), a))); }

  template<typename T> requires requires(T a) { { std::acos(std::fmin(T(1), std::fmax(T(-1), a))) }; }
    inline constexpr auto safe_acos(T a) { return std::acos(std::fmin(T(1), std::fmax(T(-1), a))); }

  template<typename T, typename U> requires std::is_scalar_v<T> && std::is_scalar_v<U>
    inline constexpr auto eq(T a, U b) { return (a==b); }

  template<typename T, typename U> requires std::is_scalar_v<T> && std::is_scalar_v<U>
    inline constexpr auto neq(T a, U b) { return (a!=b); }
  //! @}


  
  /*** Implementation detail for invoking a function on arrays ***/
  namespace detail {
    
    //! \brief Application of a tertiary function on an array and optionally other arguments
    template<typename FUNC, typename T, size_t N, typename... U> requires requires(const FUNC& func, const array<T,N>& t, const U&... u) { { func(*bbm::begin(t), (*bbm::begin(u))...) }; }
    inline auto array_invoke(const FUNC& func, const array<T,N>& t, const U&... u)
    {
      using func_t = decltype( func(*t.begin(), (*bbm::begin(u))...) );
      using result_t = replace_scalar_t< func_t, array<scalar_t<func_t>, N> >;
      result_t result;
      bbm::multirange_for([&](auto& r, auto& t, auto&... u) { r = func(t,u...); }, result, t, u...);
      return result;
    }

  } // end detail namespace


  /**********************************************************************/
  /*! \brief Helper Macro for defining functions on arrays
   **********************************************************************/
#define BBM_ARRAY_FUNC(Func)                                            \
    template<typename T, typename... U> requires requires(const T& t, const U&... u) {{backbone::detail::array_invoke([](auto&t, auto&... u) { return Func(t, u...); }, t, u...)};} \
      inline auto Func(const T& t, const U&... u)                        \
  {                                                                      \
    return backbone::detail::array_invoke([](auto& t, auto&... u) { return Func(t, u...); }, t, u...); \
  }                                                                      \
  
  //! @{ Math Functions on iterable containers
  BBM_ARRAY_FUNC( exp );
  BBM_ARRAY_FUNC( log );
  BBM_ARRAY_FUNC( sin );
  BBM_ARRAY_FUNC( asin );
  BBM_ARRAY_FUNC( cos );
  BBM_ARRAY_FUNC( acos );
  BBM_ARRAY_FUNC( tan );
  BBM_ARRAY_FUNC( atan );
  BBM_ARRAY_FUNC( sinh );
  BBM_ARRAY_FUNC( asinh );
  BBM_ARRAY_FUNC( cosh );
  BBM_ARRAY_FUNC( acosh );
  BBM_ARRAY_FUNC( tanh );
  BBM_ARRAY_FUNC( atanh );
  BBM_ARRAY_FUNC( ceil );
  BBM_ARRAY_FUNC( floor );
  BBM_ARRAY_FUNC( round );
  BBM_ARRAY_FUNC( abs );
  BBM_ARRAY_FUNC( sign );
  BBM_ARRAY_FUNC( sqrt );
  BBM_ARRAY_FUNC( cbrt );
  BBM_ARRAY_FUNC( rcp );
  BBM_ARRAY_FUNC( rsqrt );
  BBM_ARRAY_FUNC( safe_sqrt );
  BBM_ARRAY_FUNC( safe_asin );
  BBM_ARRAY_FUNC( safe_acos );

  BBM_ARRAY_FUNC( erf );
  BBM_ARRAY_FUNC( erfc );
  BBM_ARRAY_FUNC( erfinv );
  BBM_ARRAY_FUNC( tgamma );
  BBM_ARRAY_FUNC( lgamma );
  
  BBM_ARRAY_FUNC( eq );
  BBM_ARRAY_FUNC( neq );
  BBM_ARRAY_FUNC( atan2 );
  BBM_ARRAY_FUNC( max );
  BBM_ARRAY_FUNC( min );
  BBM_ARRAY_FUNC( fmod );
  BBM_ARRAY_FUNC( copysign );
  BBM_ARRAY_FUNC( pow );

  BBM_ARRAY_FUNC( lerp );
  BBM_ARRAY_FUNC( clamp );

  BBM_ARRAY_FUNC( isnan );
  BBM_ARRAY_FUNC( isinf );
  BBM_ARRAY_FUNC( isfinite );
  
  template<typename T> requires is_array_v<T> && requires(const T& t) {{ cos(t) }; { sin(t) }; }
    inline vec2d<T> cossin(const T& a)
  {
    return vec2d<T>( cos(a), sin(a) );
  }
  //! @}
  
} // end backbone namespace
  
#endif /* _BBM_NATIVE_MATH_H_ */
