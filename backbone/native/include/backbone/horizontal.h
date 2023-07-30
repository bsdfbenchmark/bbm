#ifndef _BBM_NATIVE_HORIZONTAL_H_
#define _BBM_NATIVE_HORIZONTAL_H_

#include <cmath>
#include <numeric>
#include <concepts>
#include <algorithm>
#include <functional>
#include <type_traits>

#include "backbone/math.h"

/************************************************************************/
/*! \file horizonal.h

  \brief Emulate horizontal operations on scalar and array types

*************************************************************************/

namespace backbone {
  
  //! @{ \name Horizontal operations on scalars
  template<typename T> requires std::is_scalar_v<T>
    inline T hsum(const T& t) { return t; }

  template<typename T> requires std::is_scalar_v<T>
    inline T hprod(const T& t) { return t; }

  template<typename T> requires std::is_scalar_v<T>
    inline T hmax(const T& t) { return t; }

  template<typename T> requires std::is_scalar_v<T>
    inline T hmin(const T& t) { return t; }
  
  template<typename T> requires std::is_scalar_v<T>
    inline T dot(const T& a, const T& b) { return a*b; }

  template<typename T> requires std::is_scalar_v<T>
    inline T norm(const T& t) { return std::abs(t); }

  template<typename T> requires std::is_scalar_v<T>
    inline T squared_norm(const T& t) { return t*t; }

  template<typename T> requires std::is_scalar_v<T>
    inline T normalize(const T& ) { return 1; }
  //! @}

  //! @{ \name Mask operations on scalars
  template<typename T> requires std::convertible_to<T, bool>
    inline bool all(const T& t) { return t; }

  template<typename T> requires std::convertible_to<T, bool>
    inline bool any(const T& t) { return t; }

  template<typename T> requires std::convertible_to<T, bool>
    inline bool none(const T& t) { return !t; }

  template<typename T> requires std::convertible_to<T, bool>
    inline size_t count(const T& t) { return t; }
  //! @}

  //! @{ \name Horizontal operations on arrays
  template<typename T, size_t N>
    inline auto hsum(const array<T,N>& t)
  {
    return std::accumulate(std::begin(t), std::end(t), std::decay_t<T>(0));
  }

  template<typename T, size_t N>
    inline auto hprod(const array<T,N>& t)
  {
    return std::accumulate(std::begin(t), std::end(t), std::decay_t<T>(1), std::multiplies<value_t<T>>());
  }

  template<typename T, size_t N>
    inline auto hmax(const array<T,N>& t)
  {
    return *std::max_element(std::begin(t), std::end(t));
  }

  template<typename T, size_t N>
    inline auto hmin(const array<T,N>& t)
  {
    return *std::min_element(std::begin(t), std::end(t));
  }

  template<typename T, typename U, size_t N>
    inline auto dot(const array<T,N>& a, const array<U,N>& b)
  {
    return std::inner_product(std::begin(a), std::end(a), std::begin(b), std::decay_t<T>(0));
  }

  template<typename T, size_t N>
    inline auto squared_norm(const array<T,N>& t)
  {
    return dot(t, t);
  }

  template<typename T, size_t N>
    inline auto norm(const array<T,N>& t)
  {
    return sqrt(squared_norm(t));
  }

    template<typename T, size_t N>
    inline auto normalize(const array<T,N>& t)
  {
    return t * rsqrt(squared_norm(t));
  }
  //! @}
  
  //! @{ \Name Mask operations on arrays
  template<size_t N>
    inline bool all(const array<bool,N>& t)
  {
    return std::all_of(std::begin(t), std::end(t), std::identity());
  }

  template<size_t N>
    inline bool any(const array<bool,N>& t)
  {
    return std::any_of(std::begin(t), std::end(t), std::identity());
  }    

  template<size_t N>
    inline bool none(const array<bool,N>& t)
  {
    return std::none_of(std::begin(t), std::end(t), std::identity());
  }

  template<size_t N>
    inline size_t count(const array<bool,N>& t)
  {
    return hsum(t); 
  }
  //! @}

} // end backbone namespace

#endif /* _BBM_NATIVE_HORIZONTAL_H_ */
