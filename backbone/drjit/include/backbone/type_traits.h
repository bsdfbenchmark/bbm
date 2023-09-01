#ifndef _BBM_DRJIT_TYPE_TRAITS_H_
#define _BBM_DRJIT_TYPE_TRAITS_H_

#include <type_traits>
#include "drjit/array_traits.h"

#ifdef DRJIT_AUTODIFF
  #include "drjit/autodiff.h"
#endif /* DRJIT_AUTODIFF */

/************************************************************************/
/*! \file type_traits.h

  \brief BBM type traits

  Implements: concepts::backbone::type_traits

*************************************************************************/

namespace backbone {

  /*** Implementation details for value_t; handle value_t of DiffArray differetly than DrJIT ***/
  namespace detail {
    template<typename T> struct value { using type = drjit::value_t<T>; };
    template<typename T> struct value<drjit::LLVMArray<T>> { using type = drjit::LLVMArray<T>; };
  #ifdef DRJIT_AUTODIFF
    template<typename T> struct value<drjit::DiffArray<T>> { using type = drjit::DiffArray<T>; };
  #endif /* DRJIT_AUTODIFF */
  } // end detail namespace
    
  //! @{ Direct mapping to DrJIT traits
  template<typename T> using mask_t = drjit::mask_t<T>;
  template<typename T> inline constexpr bool is_mask_v = drjit::is_mask_v<T>;

  template<typename T> using value_t = typename backbone::detail::value<std::decay_t<T>>::type;
  template<typename T> using scalar_t = drjit::scalar_t<std::decay_t<T>>;
  template<typename T, typename R> using replace_scalar_t = drjit::replace_scalar_t<std::decay_t<T>, R>;
  //! @}

  /*** Implementation details for is_diff_v ***/
  namespace detail {
    template<typename T> struct is_diff : std::false_type {};

  #ifdef DRJIT_AUTODIFF
    template<typename T> struct is_diff<drjit::DiffArray<T>> : std::true_type {};
  #endif /* DRJIT_AUTODIFF */
    
  } // end detail namespace

  
  /*** Implementation details for detecting LLVM arrays ***/
  namespace detail {

    template<typename T> struct is_LLVMArray : std::false_type {};
    template<typename T> struct is_LLVMArray<drjit::LLVMArray<T>> : std::true_type {};
  } // end detail namespace
  
  /**********************************************************************/
  /*! \brief true if the type is an LLVMArray
   **********************************************************************/
  template<typename T>
    static constexpr bool is_LLVMArray_v = detail::is_LLVMArray<std::decay_t<T>>::value;

 
  /**********************************************************************/
  /*! \brief true if type supports autodiff
    *********************************************************************/
  template<typename T> inline constexpr bool is_diff_v = std::disjunction< backbone::detail::is_diff<std::decay_t<T>>, backbone::detail::is_diff<drjit::value_t<std::decay_t<T>>>>::value;

  /**********************************************************************/
  /*! \brief true if DiffArray type
   **********************************************************************/
  template<typename T> inline constexpr bool is_DiffArray_v = backbone::detail::is_diff<std::decay_t<T>>::value;

    
  /*** Implementation details for remove_diff_t ***/
  namespace detail {
    template<typename T> struct remove_diff { using type = T; };

  #ifdef DRJIT_AUTODIFF
    template<typename T> struct remove_diff<drjit::DiffArray<T>> { using type = T; };
  #endif /* DRJIT_AUTODIFF */

    template<typename T> requires (drjit::is_static_array_v<T> && !detail::is_diff<T>::value)
      struct remove_diff<T>
    {
      using nodiff_value_t = typename remove_diff<drjit::value_t<T>>::type;
      using type = typename T::template ReplaceValue< nodiff_value_t >;
    };
  } // end detail namespace
  
  /**********************************************************************/
  /*! \brief Strip autodiff from type T.
   **********************************************************************/
  template<typename T> using remove_diff_t = typename detail::remove_diff<std::decay_t<T>>::type;

  
  /*** Implementation details for add_diff_t ***/
  namespace detail {
    template<typename T> struct add_diff;
      
  #ifdef DRJIT_AUTODIFF
    template<typename T> struct add_diff { using type = drjit::DiffArray<remove_diff_t<T>>; };

    template<typename T> requires drjit::is_static_array_v<T>
      struct add_diff<T>
    {
      using diff_value_t = typename add_diff<drjit::value_t<T>>::type;
      using type = typename T::template ReplaceValue< diff_value_t >;
    };
    #else /* NOT DRJIT_AUTODIFF */
    template<typename T> struct add_diff { using type = T; static_assert( bbm::dependent_false_v<T>, "BBM: DRJIT backbone compiled without autodiff."); };
    #endif /* DRJIT_AUTODIFF */
  } // end detail namespace

  /********************************************************************/
  /*! \brief Add autodiff to type T.
   ********************************************************************/
  template<typename T>
  using add_diff_t = typename detail::add_diff<std::decay_t<T>>::type;

  
  /*** Implementation details for is_packet_v ***/
  namespace detail {
    template<typename T> struct is_packet : std::false_type {};
    template<typename T> struct is_packet<drjit::Packet<T>> : std::true_type {};
  } // end detail namespace

  /**********************************************************************/
  /*! \brief true is the type is a packet
   **********************************************************************/
  template<typename T>
    inline constexpr bool is_packet_v = std::disjunction< detail::is_packet<std::decay_t<T>>, detail::is_packet<value_t<std::decay_t<T>>> >::value;

  
  /*** Implementation detal for remove_packet_t ***/
  namespace detail {
    template<typename T> struct remove_packet { using type = T; };
    template<typename T> struct remove_packet<drjit::Packet<T>> { using type = T; };
    template<typename T> struct remove_packet<drjit::PacketMask<T>> { using type = T; };
    
    template<typename T> requires (drjit::is_static_array_v<T> && !is_packet<T>::value)
      struct remove_packet<T>
    {
      using nopacket_value_t = typename remove_packet<drjit::value_t<T>>::type;
      using type = typename T::template ReplaceValue< nopacket_value_t >;
    };
  }// end detail namespace

  /**********************************************************************/
  /*! \brief Strip packet type from T.
   **********************************************************************/
  template<typename T>
    using remove_packet_t = typename detail::remove_packet<std::decay_t<T>>::type;

  
  /*** Implementation details for add_packet_t ***/
  namespace detail {
    template<typename T> struct add_packet { using type = drjit::Packet<remove_packet_t<T>>; };
    template<typename T> requires  drjit::is_mask_v<T>
      struct add_packet<T> { using type = drjit::Mask<remove_packet_t<T>>; };
    
    template<typename T> requires drjit::is_static_array_v<T>
      struct add_packet<T>
    {
      using packet_value_t = typename add_packet<drjit::value_t<T>>::type;
      using type = typename T::template ReplaceValue< packet_value_t >;
    };
  } // end detail namespace
  
  /********************************************************************/
  /*! \brief Add packet to type T. 
   ********************************************************************/
  template<typename T>
    using add_packet_t = typename detail::add_packet<std::decay_t<T>>::type;


  /*** Implementation details for index_t and is_index_v ***/
  namespace detail {
    template<typename T> struct is_index : std::is_convertible<T, size_t> {};
    template<> struct is_index<drjit::Packet<size_t>> : std::true_type {};
    template<> struct is_index<drjit::LLVMArray<size_t>> : std::true_type {};
    
    template<typename T>
      struct index_impl
    {
      using type = std::conditional_t<is_LLVMArray_v<T>, drjit::LLVMArray<size_t>, size_t>;
      using mask = std::conditional_t<is_LLVMArray_v<T>, drjit::LLVMArray<bool>, bool>; 
    };

    template<typename T> requires is_packet_v<T>
      struct index_impl<T>
    {
      using type = drjit::Packet<size_t>;
      using mask = mask_t<drjit::Packet<scalar_t<T>>>;
    };
  } // end detail namespace
  
  /********************************************************************/
  /*! \brief Return the index type 
   ********************************************************************/
  template<typename T>
    using index_t = typename detail::index_impl<std::decay_t<T>>::type;

  /********************************************************************/
  /*! \brief true if the type is an index type
   ********************************************************************/
  template<typename T>
    inline constexpr bool is_index_v = detail::is_index<std::decay_t<T>>::value;

  /********************************************************************/
  /*! \brief Return the mask of an index of a type.
   ********************************************************************/
  template<typename T>
    using index_mask_t = typename detail::index_impl<std::decay_t<T>>::mask;


} // end backbone namespace

#endif /* _BBM_DRJIT_TYPE_TRAITS_H_ */
