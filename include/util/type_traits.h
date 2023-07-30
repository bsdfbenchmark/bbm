#ifndef _BBM_TYPETRAITSUTIL_H_
#define _BBM_TYPETRAITSUTIL_H_

#include <vector>
#include <string>
#include <functional>
#include <type_traits>
#include "concepts/util.h"

/***********************************************************************/
/*! \file type_traits_util.h
    \brief Extensions to the STL type traits
************************************************************************/ 

namespace bbm {

  /*********************************************************************/
  /*! @{ \name is_const (reference robust)
   *********************************************************************/
  template<typename T>
    using is_const = std::is_const<std::remove_reference_t<std::remove_pointer_t<T>>>;

  template<typename T>
    inline constexpr bool is_const_v = is_const<T>::value;
  //! @}

  /*********************************************************************/
  /*! @{ \name add_const (reference and pointer robust)
   *********************************************************************/
  template<typename T> struct add_const { using type = std::add_const_t<T>; };

  template<typename T> requires std::is_lvalue_reference_v<T>
    struct add_const<T> { using type = std::add_lvalue_reference_t<typename bbm::add_const<std::remove_reference_t<T>>::type>; };

  template<typename T> requires std::is_rvalue_reference_v<T>
    struct add_const<T> { using type = std::add_rvalue_reference_t<typename bbm::add_const<std::remove_reference_t<T>>::type>; };

  template<typename T> requires std::is_pointer_v<T>
    struct add_const<T> { using type = std::add_pointer_t<typename bbm::add_const<std::remove_pointer_t<T>>::type>; };

  template<typename T>
    using add_const_t = typename add_const<T>::type;
  //! @}
  
  /*********************************************************************/
  /*! \@{ \name remove_const (reference and pointer robust)
   *********************************************************************/
  template<typename T> struct remove_const { using type = std::remove_const_t<T>; };

  template<typename T> requires std::is_lvalue_reference_v<T>
    struct remove_const<T> { using type = std::add_lvalue_reference_t<typename bbm::remove_const<std::remove_reference_t<T>>::type>; };

  template<typename T> requires std::is_rvalue_reference_v<T>
    struct remove_const<T> { using type = std::add_rvalue_reference_t<typename bbm::remove_const<std::remove_reference_t<T>>::type>; };

  template<typename T> requires std::is_pointer_v<T>
    struct remove_const<T> { using type = std::add_pointer_t<typename bbm::remove_const<std::remove_pointer_t<T>>::type>; };
  
  template<typename T>
    using remove_const_t = typename remove_const<T>::type;
  //! @}
  

  /*** Implementation details for find_first ***/
  namespace detail {
    template<template<typename> typename TRAIT, typename F, typename... Ts> struct find_first_impl { using type = F; };
    
    template<template<typename> typename TRAIT, typename F, typename T0, typename... Ts> requires (!TRAIT<F>::value && TRAIT<T0>::value)
      struct find_first_impl<TRAIT, F, T0, Ts...> 
    {
      using type = T0;
    };
    
    template<template<typename> typename TRAIT, typename F, typename T0, typename... Ts> requires (!TRAIT<F>::value && !TRAIT<T0>::value)
      struct find_first_impl<TRAIT, F, T0, Ts...> 
    {
      using type = typename find_first_impl<TRAIT, F, Ts...>::type;
    };
  }
  
  /**********************************************************************/
  /*! \brief Find first type that matches a type_trait
    *********************************************************************/
  template<template<typename> typename TRAIT, typename... Ts>
    using find_first = typename bbm::detail::find_first_impl<TRAIT, void, Ts...>::type;

  
  /*** implementation details for string_type ***/
  namespace detail {
    template<typename T> struct is_string_type_impl : std::false_type {};

    template<size_t N> struct is_string_type_impl< char[N] > : std::true_type {};
    template<> struct is_string_type_impl<char*> : std::true_type {};
    template<> struct is_string_type_impl<const char*> : std::true_type {};
    template<> struct is_string_type_impl<std::string> : std::true_type {};
  }

  /*********************************************************************/
  /*! @{ \name type-trait for detecting string_types: char*, char[], string
   *********************************************************************/
  template<typename T>
    using is_string_type = bbm::detail::is_string_type_impl<std::remove_const_t<std::decay_t<T>>>;
  
  template<typename T>
    inline constexpr bool is_string_type_v = is_string_type<T>::value;
  //! @}
  
  
  /*** Implementation Details ***/
  namespace detail {
    template<typename T>
      struct is_vector_impl : std::false_type {};

    template<typename T>
      struct is_vector_impl<std::vector<T>> : std::true_type {};
  }

  /*********************************************************************/
  /*! @{ \name type-trait for detecting vectors
   *********************************************************************/
  template<typename T>
    using is_vector = bbm::detail::is_vector_impl<std::decay_t<T>>;

  template<typename T>
    inline constexpr bool is_vector_v = is_vector<T>::value;
  //! @}
  
  
  /*** Implementation Details ***/
  namespace detail {
    template<typename T>
      struct is_tuple_impl : std::false_type {};

    template<typename... Ts>
      struct is_tuple_impl<std::tuple<Ts...>> : std::true_type {};
  }

  /*********************************************************************/
  /*! @{ \name type-trait for detecting tuples
   *********************************************************************/ 
  template<typename T>
    using is_tuple = bbm::detail::is_tuple_impl<std::decay_t<T>>;

  template<typename T>
    inline constexpr bool is_tuple_v = is_tuple<T>::value;
  //! @}
  


  /*** Implementation details ***/
  namespace detail {
    template<typename T>
      struct is_array_impl : std::false_type {};

    template<typename T, size_t N>
      struct is_array_impl<std::array<T,N>> : std::true_type {};
  }
  
  /**********************************************************************/
  /*! @{ \name type-trait for detecting arrays
   **********************************************************************/
  template<typename T>
    using is_array = bbm::detail::is_array_impl<std::decay_t<T>>;

  template<typename T>
    inline constexpr bool is_array_v = is_array<T>::value;
  //! @}

} // end bbm namespace

#endif /* _BBM_TYPETRAITSUTIL_H_ */
