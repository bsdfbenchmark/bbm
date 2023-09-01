#ifndef _BBM_NATIVE_TYPE_TRAITS_H_
#define _BBM_NATIVE_TYPE_TRAITS_H_

#include "concepts/util.h"
#include "backbone/array.h"
#include "backbone/complex.h"
#include "core/error.h"

/************************************************************************/
/*! \file type_traits.h

  \brief Type traits
  
  Implements: concepts::backbone::type_traits
  
*************************************************************************/

namespace backbone {
  
  /*** Implementation details for value_t ***/
  namespace detail {
    template<typename T> struct value_impl { using type = T; };
    template<typename T, size_t N> struct value_impl<array<T,N>> { using type = T; };
    template<typename T> struct value_impl<complex<T>> { using type = T; };
  } // en detail namespace
    
  //! \brief Value trait
  template<typename T>
    using value_t = typename detail::value_impl<std::decay_t<T>>::type;

  /*** Implementation details for scalar_t ***/
  namespace detail {
    template<typename T> struct scalar_impl { using type = T; };
    template<typename T, size_t N> struct scalar_impl<array<T,N>> { using type = typename scalar_impl<std::decay_t<T>>::type; };
    template<typename T> struct scalar_impl<complex<T>> { using type = typename scalar_impl<std::decay_t<T>>::type; };
  } // en detail namespace
  
  //! \brief Scalar trait
  template<typename T>
    using scalar_t = typename detail::scalar_impl<std::decay_t<T>>::type;

  /*** Implementation details for replace_value_t ***/
  namespace detail {
    template<typename T, typename R> struct replace_scalar { using type = R; };
    template<typename T, size_t N, typename R> struct replace_scalar<array<T,N>, R> { using type = array<R,N>; };

    template<typename T, typename R> requires std::floating_point<R> struct replace_scalar<complex<T>, R> { using type = complex<R>; };
    template<typename T, typename R> requires (!std::floating_point<R>) struct replace_scalar<complex<T>, R> { using type = array<R,2>; };
  }
  
  //! \brief Replace_scalar trait
  template<typename T, typename R>
    using replace_scalar_t = typename detail::replace_scalar<std::decay_t<T>, R>::type;
  
  //! @{ \name Differentiability traits
  template<typename T>
    inline constexpr bool is_diff_v = false;

  template<typename T>
    using remove_diff_t = std::decay_t<T>;
 
  template<typename T>
    struct add_diff_t { static_assert( bbm::dependent_false_v<T>, "BBM: native backbone does not support autodiff."); };
  //! @}

  //! @{ \name Packet traits
  template<typename T>
    inline constexpr bool is_packet_v = false;

  template<typename T>
    using remove_packet_t = std::decay_t<T>;

  template<typename T>
    struct add_packet_t { static_assert( bbm::dependent_false_v<T>, "BBM: native backbone does not support packets."); };

  //! @}
  
  //! @{ \name Mask traits
  template<typename T>
    using mask_t = replace_scalar_t<T, bool>;

  template<typename T>
    inline constexpr bool is_mask_v = std::convertible_to<T, mask_t<T>>;
  //! @}

  //! @{ \name Index traits
  template<typename T>
    using index_t = size_t;

  template<typename T>
    using index_mask_t = bool;
  
  template<typename T>
    inline constexpr bool is_index_v = std::convertible_to<T, index_t<T>>;
  //! @}

  
} // end backbone namespace

#endif /* _BBM_NATIVE_TYPE_TRAITS_H_ */

