#ifndef _BBM_ENOKI_RANDOM_H_
#define _BBM_ENOKI_RANDOM_H_

#include <limits>
#include <cfloat>
#include "enoki/random.h"

#include "backbone/type_traits.h"

/************************************************************************/
/*! \filename random.h

  \brief Random number generator; built on top of Enoki

*************************************************************************/

namespace backbone {

  /**********************************************************************/
  /*! \brief seed type
   **********************************************************************/
  using seed_t = uint64_t;
  
  /**********************************************************************/
  /*! \bref Default seed
  ***********************************************************************/
  static constexpr seed_t default_seed = PCG32_DEFAULT_STATE;
  
  /**********************************************************************/
  /*! \brief Random generator wrapper around Enoki's PCG32
  ***********************************************************************/
  template<typename T>
    struct rng
  {
    using type = T;

    //! @{ Constructors
    inline rng(void)
    {
      _rng(default_seed);
      _lower = 0;
      if constexpr (std::integral<scalar_t<T>>) _upper = std::numeric_limits<T>::max();
      else _upper = 1;
    }

    inline rng(seed_t seed) : rng()
    {
      _rng.seed(seed);
    }
    
    inline rng(type lower, type upper) : _lower(lower), _upper(upper), _rng(default_seed) {}
    inline rng(seed_t seed, type lower, type upper) : _lower(lower), _upper(upper), _rng(seed) {}
    //! @}

    //! \brief set the seed
    inline void seed(auto seed)
    {
      _rng.seed(seed);
    }

    //!\ brief get next number
    inline type operator()(void)
    {
      // float
      if constexpr (std::floating_point<scalar_t<T>>)
      {
        if constexpr  (std::numeric_limits<scalar_t<T>>::digits > FLT_MANT_DIG) return _rng.next_float64() * (_upper - _lower) + _lower;
        else return _rng.next_float32() * (_upper - _lower) + _lower;
      }

      // integral
      else //if constexpr (std::integral<scalar_t<T>>)
      {
        if constexpr (std::numeric_limits<scalar_t<T>>::digits > 32) return _rng.next_uint64_bounded((_upper - _lower)) + _lower;
        else return _rng.next_uint32_bounded((_upper - _lower)) + _lower;
      }

    }

  private:
    enoki::PCG32<T> _rng;
    type _upper, _lower;
  };
  
} // end backbone namespace

#endif /* _BBM_ENOKI_RANDOM_H_ */

