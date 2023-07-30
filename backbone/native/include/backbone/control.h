#ifndef _BBM_NATIVE_CONTROL_H_
#define _BBM_NATIVE_CONTROL_H_

#include <concepts>

#include "util/multirange_for.h"

#include "backbone/type_traits.h"
#include "backbone/horizontal.h"

/************************************************************************/
/*! \file control.h

  \brief Data and flow control
  + cast
  + select
  + lookup
  
*************************************************************************/

namespace backbone {

  /*********************************************************************/
  /*! \brief cast
   *********************************************************************/
  template<typename NEWTYPE, typename OLDTYPE>
    inline constexpr NEWTYPE cast(OLDTYPE&& val)
  {
    return static_cast<NEWTYPE>(val);
  }
  
  /*********************************************************************/
  /*! @{ \name select 
   *********************************************************************/
  template<typename T, typename U>  requires requires(std::decay_t<T> a, std::decay_t<U> b) {{a+b};}
    inline constexpr auto select(bool mask, const T& a, const U& b)
  {
    using return_type = decltype( std::declval<std::decay_t<U>>() + std::declval<std::decay_t<T>>() );
    return (mask ? return_type(a) : return_type(b));
  }

  template<typename T, typename U, size_t N>  requires requires(value_t<T> a, value_t<U> b) {{a+b};}
    inline constexpr auto select(const array<bool,N>& mask, const T& a, const U& b)
  {
    using return_type = decltype( std::declval<value_t<U>>() + std::declval<value_t<T>>() );
    
    array<return_type, N> result;
    bbm::multirange_for( [](auto& r, auto& m, auto& a, auto& b) {  r = select(m,a,b); }, result, mask, a, b);
    return result;
  }

  template<typename T, typename U> requires (!is_complex_v<U>) && requires(value_t<T> a, value_t<U> b) {{a+b};}
    inline constexpr auto select(const array<bool,2>& mask, const complex<T>& z, const U& u)
  {
    return select(mask, z, complex(u));
  }

  template<typename T, typename U> requires (!is_complex_v<U>) && requires(value_t<T> a, value_t<U> b) {{a+b};}
    inline constexpr auto select(const array<bool,2>& mask, const U& u, const complex<T>& z)
  {
    return select(mask, complex(u), z);
  }
  //! @}
  
  /**********************************************************************/
  /*! \brief lookup
   **********************************************************************/
  template<typename RET, typename C> requires std::ranges::range<C> && std::convertible_to<bbm::iterable_value_t<C>, RET>
    inline constexpr RET lookup(C&& container, size_t idx, bool mask=true)
  {
    // quick bail out
    if(!mask) return RET();

    // lookup
    if(idx >= bbm::size(container)) throw bbm_out_of_range;
    return RET(*(std::next(bbm::begin(container), idx)));
  }

  /**********************************************************************/
  /*! \brief set
   **********************************************************************/
  template<typename VAL, typename C> requires std::ranges::range<C> && std::convertible_to<VAL, bbm::iterable_value_t<C>>
    inline constexpr void set(C&& container, size_t idx, VAL&& value, bool mask=true)
  {
    // quick bail out
    if(!mask) return;

    // set
    if(idx >= bbm::size(container)) throw bbm_out_of_range;
    *(std::next(bbm::begin(container), idx)) = std::forward<VAL>(value);
  }
  
  /**********************************************************************/
  /*! \brief binary_search
   **********************************************************************/
  template<typename C, typename PRED> requires std::ranges::range<C> && std::is_invocable_r_v<bool, PRED, bbm::iterable_value_t<C>>
    inline constexpr size_t binary_search(C&& container, PRED&& predicate, bool mask=true)
  {
    using value_type = bbm::iterable_value_t<C>;

    // quick exit
    if(!mask) return bbm::size(container);
    
    // create a wrapper for the predicate to meet stl's expectations
    auto pred_wrapper = [&](const value_type& val, const value_type& )
    {
      return predicate(val);
    };

    // pass control to stl
    return std::distance( bbm::begin(container), std::lower_bound(bbm::begin(container), bbm::end(container), value_type(), pred_wrapper) );
  }
  
} // end backbone namespace

#endif /* _BBM_NATIVE_CONTROL_H_ */
