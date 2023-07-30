#ifndef _BBM_NATIVE_RANDOM_H_
#define _BBM_NATIVE_RANDOM_H_

#include <limits>
#include <random>

#include "backbone/type_traits.h"
#include "backbone/array.h"
#include "backbone/control.h"

#include "util/multirange_for.h"

/************************************************************************/
/*! \filename random.h

  \brief Random number generator; built on top of STL

*************************************************************************/

namespace backbone {

  /**********************************************************************/
  /*! \brief seed type
   **********************************************************************/
  using seed_t = std::uint_fast64_t;
    
  /**********************************************************************/
  /*! \brief Default seed
   **********************************************************************/
  static constexpr seed_t default_seed = std::mt19937_64::default_seed;
  
  /**********************************************************************/
  /*! \brief Forward declaration; specialized below
   **********************************************************************/
  template<typename T> struct rng;

  /**********************************************************************/
  /*! \brief Integral specialization
   **********************************************************************/
  template<typename T> requires std::integral<T>
    struct rng<T>
  {
    using type = T;

    //@{ \name Constructors
    inline rng(void) : _rng(default_seed) {}
    inline rng(seed_t seed, type lower=0, type upper=std::numeric_limits<T>::max()) : _dist(lower, upper), _rng(seed) {}
    inline rng(type lower, type upper) : _dist(lower, upper), _rng(default_seed) {}
    //! @}

    //! \brief set the seed
    inline void seed(seed_t seed)
    {
      _rng.seed(seed);
    }
    
    //! \brief get next number
    inline type operator()(void)
    {
      return _dist(_rng);
    }
    
  private:
    std::mt19937_64 _rng;
    std::uniform_int_distribution<T> _dist;
  };

  /**********************************************************************/
  /*! \brief Floating point specialization
   **********************************************************************/
  template<typename T> requires std::floating_point<T>
    struct rng<T>
  {
    using type = T;

    //! @{ Constructors
    inline rng(void) {}
    inline rng(auto seed, type lower=0, type upper=1) : _dist(lower, upper), _rng(seed) {}
    inline rng(type lower, type upper) : _dist(lower, upper) {}
    //! @}

    //! \brief set the seed
    inline void seed(auto seed)
    {
      _rng.seed(seed);
    }
    
    //! \brief get next number
    inline type operator()(void)
    {
      return _dist(_rng);
    }
    
  private:
    std::mt19937_64 _rng;
    std::uniform_real_distribution<T> _dist;
  };

  /**********************************************************************/
  /*! \brief Array specialization
   **********************************************************************/
  template<typename T> requires is_array_v<T>
    struct rng<T> : public rng<scalar_t<T>>
  {
    using type = T;

    //! Constructor passthrough
    using rng<scalar_t<T>>::rng;

    //! \brief get next number
    inline type operator()(void)
    {
      type result;
      bbm::multirange_for([&](auto& r)
      {
        r = static_cast<rng<scalar_t<T>>&>(*this)();
      }, result);
      return result;
    }
  };
  
} // end backbone namespace

#endif /* _BBM_NATIVE_RANDOM_H_ */

