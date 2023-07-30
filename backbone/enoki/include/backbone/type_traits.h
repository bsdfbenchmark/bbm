#ifndef _BBM_ENOKI_TYPE_TRAITS_H_
#define _BBM_ENOKI_TYPE_TRAITS_H_

#include <type_traits>
#include "enoki/array_traits.h"

#ifdef ENOKI_AUTODIFF
  #include "enoki/autodiff.h"
#endif /* ENOKI_AUTODIFF */

/************************************************************************/
/*! \file type_traits.h

  \brief Bbm type traits

  Implements: concepts::backbone::type_traits

*************************************************************************/

namespace backbone {

    /*** Implementation details for mask_t ***/
    namespace detail {

    #ifdef ENOKI_AUTODIFF
      //! \brief bbm_mask is a specialization of enoki::DiffArray<bool>.  This
      //!        specialization is needed because enoki::DiffArray<bool> does
      //!        not play nice with pybind11.  The specific issue is that the
      //!        additional parameters pybind passes are being captured by the
      //!        constructor of enoki:DiffArray<bool>.  The solution is a
      //!        specialization with a restricted constructor that only takes
      //!        a bool without any additional flags.
      struct bbm_mask : public enoki::DiffArray<bool>
      {
        using base = enoki::DiffArray<bool>;

        //! @{ \name Constructors
        bbm_mask(void) : base() {};
        bbm_mask(bool b) : base(b) {};
        bbm_mask(const base& m) : base(m) {};
        bbm_mask(const base& m, enoki::detail::reinterpret_flag) : base(m) {} // ignore flags
        bbm_mask(base&& m) : base(std::forward<base>(m)) {}
        bbm_mask(const enoki::Array<bool, 1>& d) : base(d[0]) {}
        bbm_mask(const enoki::Array<bool, 1>& d, enoki::detail::reinterpret_flag) : base(d[0]) {} // ignore flags
        //! @}

        //! @{ \name cast operator
        operator bool(void) const { return enoki::detach(*this); }
        //! @}
      };
    #endif /* ENOKI_AUTODIFF */

      //! @{ \name Helper type traits
      template<typename T> struct is_mask : enoki::is_mask<T> {};

      template<typename T> struct mask { using type = enoki::mask_t<T>; };
      template<> struct mask<bbm_mask> { using type = bbm_mask; };
      
    #ifdef ENOKI_AUTODIFF
        template<> struct is_mask<bbm_mask> : std::true_type {};

        template<typename T> requires std::same_as<enoki::mask_t<T>, enoki::DiffArray<bool>>
          struct mask<T>
        {
          using type = bbm_mask;
        };

    #endif /* ENOKI_AUTODIFF */
      //! @}
    } 

    template<typename T> using mask_t = typename detail::mask<std::decay_t<T>>::type;
    template<typename T> inline constexpr bool is_mask_v = detail::is_mask<std::decay_t<T>>::value;

    
    /*** Implementation details for value_t; handle value_t of DiffArray differently then Enoki ***/
    namespace detail {
      template<typename T> struct value { using type = enoki::value_t<T>; };
    #ifdef ENOKI_AUTODIFF
      template<typename T> struct value<enoki::DiffArray<T>> { using type = enoki::DiffArray<T>; };
      template<> struct value<detail::bbm_mask> { using type = enoki::DiffArray<bool>; };
    #endif /* ENOKI_AUTODIFF */
    }
    
    //! @{ \name Direct mapping to enoki traits
    template<typename T> using value_t = typename backbone::detail::value<std::decay_t<T>>::type;
    template<typename T> using scalar_t = enoki::scalar_t<std::decay_t<T>>;
    template<typename T, typename R> using replace_scalar_t = enoki::replace_scalar_t<std::decay_t<T>,R>;
    //! @}


    /*** Implementation details for is_diff_v ***/
    namespace detail {

      template<typename T> struct is_diff : std::false_type {};

    #ifdef ENOKI_AUTODIFF
      template<typename T> struct is_diff<enoki::DiffArray<T>> : std::true_type {};
      template<> struct is_diff<detail::bbm_mask> : std::true_type {};
    #endif /* ENOKI_AUTODIFF */
      
    } // end detail namespace

    /********************************************************************/
    /*! \brief true if the type supports autodiff
      *******************************************************************/
    template<typename T>
      inline constexpr bool is_diff_v = std::disjunction< detail::is_diff<std::decay_t<T>>, detail::is_diff<enoki::value_t<std::decay_t<T>>> >::value;

    
    /*** Implementation details for remove_diff_t ***/
    namespace detail {
      template<typename T> struct remove_diff { using type = T; };

    #ifdef ENOKI_AUTODIFF
      template<typename T> struct remove_diff<enoki::DiffArray<T>> { using type = T; };
      template<> struct remove_diff<detail::bbm_mask> { using type = bool; };
    #endif /* ENOKI_AUTODIFF */

      template<typename T> requires (enoki::is_static_array_v<T> && !detail::is_diff<T>::value)
        struct remove_diff<T>
      {
        using nodiff_value_t = typename remove_diff<enoki::value_t<T>>::type;
        using type = typename T::template ReplaceValue< nodiff_value_t >;
      };

      
    } // end detail namespace

    /********************************************************************/
    /*! \brief Strip autodiff from type T.
     ********************************************************************/
    template<typename T>
      using remove_diff_t = typename detail::remove_diff<std::decay_t<T>>::type;

  
    /*** Implementation details for add_diff_t ***/
    namespace detail {
      template<typename T> struct add_diff;
      
    #ifdef ENOKI_AUTODIFF
      template<typename T> struct add_diff { using type = enoki::DiffArray<remove_diff_t<T>>; };
      template<> struct add_diff<bool> { using type = detail::bbm_mask; };
      template<> struct add_diff<detail::bbm_mask> { using type = detail::bbm_mask; };

      template<typename T> requires enoki::is_static_array_v<T>
        struct add_diff<T>
      {
        using diff_value_t = typename add_diff<enoki::value_t<T>>::type;
        using type = typename T::template ReplaceValue< diff_value_t >;
      };
    #else /* NOT ENOKI_AUTODIFF */
      template<typename T> struct add_diff { using type = T; static_assert( bbm::dependent_false_v<T>, "BBM: Enoki backbone compiled without autodiff."); };
    #endif /* ENOKI_AUTODIFF */
    } // end detail namespace

    /********************************************************************/
    /*! \brief Add autodiff to type T.
     ********************************************************************/
    template<typename T>
      using add_diff_t = typename detail::add_diff<std::decay_t<T>>::type;

  
    /*** Implementation details for is_packet_v ***/
    namespace detail {
      template<typename T> struct is_packet : std::false_type {};
      template<typename T> struct is_packet<enoki::Packet<T>> : std::true_type {};
      template<typename T> struct is_packet<enoki::PacketMask<T>> : std::true_type {};
    } // end detail namespace

    /********************************************************************/
    /*! \brief true if the type is a packet
      *******************************************************************/
    template<typename T>
      inline constexpr bool is_packet_v = std::disjunction< detail::is_packet<std::decay_t<T>>, detail::is_packet<enoki::value_t<std::decay_t<T>>> >::value;


    /*** Implementation details for remove_packet_t ***/
    namespace detail {
      template<typename T> struct remove_packet { using type = T; };
      template<typename T> struct remove_packet<enoki::Packet<T>> { using type = T; };
      template<typename T> struct remove_packet<enoki::PacketMask<T>> { using type = bool; };

      template<typename T> requires (enoki::is_static_array_v<T> && !is_packet<T>::value)
        struct remove_packet<T>
      {
        using nopacket_value_t = typename remove_packet<enoki::value_t<T>>::type;
        using type = typename std::decay_t<T>::template ReplaceValue< nopacket_value_t >;
      };  
    } // end detail namespace

    /********************************************************************/
    /*! \brief Strip packet from type T.
     ********************************************************************/
    template<typename T>
      using remove_packet_t = typename detail::remove_packet<std::decay_t<T>>::type;


    /*** Implementation details for add_packet_t ***/
    namespace detail {
      template<typename T> struct add_packet { using type = enoki::Packet<remove_packet_t<T>>; };

      template<typename T> requires enoki::is_static_array_v<T>
        struct add_packet<T>
      {
        using packet_value_t = typename add_packet<enoki::value_t<T>>::type;
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
      template<> struct is_index<enoki::Packet<size_t>> : std::true_type {};
      
      template<typename T>
        struct index_impl
      {
        using type = size_t;
        using mask = bool; 
      };
      
      template<typename T> requires is_packet_v<T>
        struct index_impl<T>
      {
        using type = enoki::Packet<size_t>;
        using mask = mask_t<enoki::Packet<scalar_t<T>>>;
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


#endif /* _BBM_ENOKI_TYPE_TRAITS_H_ */
