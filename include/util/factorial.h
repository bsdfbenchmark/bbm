#ifndef _BBM_FACTORIAL_H_
#define _BBM_FACTORIAL_H_

#include <limits>
#include <functional>
#include "core/error.h"

/************************************************************************/
/*! \file factorial.h

  \brief Compute the factorial (uses precomputation to speed up run time
  querries).

************************************************************************/

namespace bbm {

  /*** Implementation detail for determing the largest value v for which: (v--)++ = v ***/
  namespace detail {

    template<typename T> requires std::integral<T>
      constexpr T max_int(void)
    {
      return std::numeric_limits<T>::max();
    }

    template<typename T> requires std::floating_point<T>
      constexpr T max_int(void)
    {
      return (1ull << (std::numeric_limits<T>::digits));
    }
    
  };
  
  /**********************************************************************/
  /*! \brief Returns the largest possible factorial index for a given type
      that does not result in an overflow.

      \tparam T = type to compute factorial in (default = unsigned long long int)
      \returns max factorial index
   **********************************************************************/
  template<typename T=unsigned long long int>  requires std::integral<T> || std::floating_point<T>
    consteval size_t max_factorial(void)
  {
    size_t n=1;
    T fac = bbm::detail::max_int<T>();
    while(fac > n) fac /= n++;
    return n;
  }
  

  /*** Precompute at compile time a table of factorials ***/
  namespace precomputed {

    static constexpr auto factorials = []<size_t... N>(std::index_sequence<N...>)
    {
      unsigned long long int acc=1;
      return std::array<unsigned long long int, max_factorial<unsigned long long int>()+2>{ 1, (acc*=(N+1))...};          // 0! is hardcoded as 1; also include max_factorial (thus length = +2)
    }(std::make_index_sequence<max_factorial<unsigned long long int>()+1>{});

  }
  
  /********************************************************************/
  /*! \brief Compute n!

    \tparam T = type to return (default = unsigned long long int)
    \returns n!
    
    Throws an exception if n > max_factorial<T>.
    
  *********************************************************************/
  template<typename T=unsigned long long int>
    constexpr T factorial(size_t n)
  {
    // check range
    if (n > max_factorial<T>()) throw bbm_out_of_range;

    // return precomputed value from array
    return T(precomputed::factorials[n]);
  }  
  
} // end bbm namespace

#endif /* _BBM_FACTORIAL_H_ */
